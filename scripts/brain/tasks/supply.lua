local data     = require("brain.data.combat")
local items    = require("brain.data.items")
local shops    = require("brain.data.shops")
local danger   = require("brain.policies.danger")
local ground   = require("brain.policies.ground")
local memo     = require("brain.policies.memo")
local near     = require("brain.policies.near")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")
local value    = require("brain.policies.value")

local learn = require("brain.tasks.learn")

local M = {}

local trips = {}
local needs = {}

local function carried(snap, kind)
    local total = 0
    for _, dose in ipairs(recovery.doses(snap, kind)) do
        total = total + dose.amount * dose.count
    end
    local heal = recovery.heal(snap)
    if kind == "hp" and heal ~= nil and heal.hp > 0 and heal.mp > 0 then
        total = total + heal.hp / heal.mp * (snap.self.mp or 0)
    end
    return total
end

local function offers(snap, kind)
    local found = {}
    for name, shop in pairs(shops) do
        local effect = items[name]
        local amount = effect ~= nil and recovery.restored(snap, effect, kind) or 0
        if amount > 0 and shop.price <= (snap.self.money or 0) then
            local each = amount * (shop.uses or 1)
            table.insert(found, {
                name = name,
                each = each,
                price = shop.price,
                capacity = shop.capacity,
                npcs = shop.npcs,
                unit = shop.price / each,
                slot = each * shop.capacity
            })
        end
    end
    table.sort(found, function(left, right)
        return left.name < right.name
    end)
    return found
end

local function nearest(snap, query, kind, found)
    local names = {}
    for _, offer in ipairs(found) do
        table.insert(names, offer.name)
    end
    local perilous = danger.perilous(snap, query)
    local shunned  = danger.shunned(snap, query)
    local base     = table.concat({ score.signature(snap, ""), snap.world.map, table.concat(shunned, ","), #perilous },
        "|")
    local key      = table.concat({ score.signature(snap), kind, table.concat(names, ",") }, "|")
    return memo.get(trips, snap.self.name, base, key, function()
        local maps = {}
        for _, offer in ipairs(found) do
            for _, spot in ipairs(query.npcs(offer.npcs)) do
                table.insert(maps, spot.map)
            end
        end
        local route = #maps > 0 and query.route(maps, perilous, shunned) or nil
        return route ~= nil and { dest = route.dest, cost = route.cost } or nil
    end)
end

local function sold(query, offer, map)
    for _, spot in ipairs(query.npcs(offer.npcs)) do
        if spot.map == map then
            return true
        end
    end
    return false
end

local function evaluate(snap, query, kind)
    if kind == "mp" then
        if recovery.least(snap) == nil then
            return nil
        elseif recovery.rates(snap).mp > 0 then
            local spare = recovery.ignition(snap)
            return spare > 0 and { crisis = spare, stock = recovery.spare(snap), loot = 0 } or nil
        end
        local refill = ground.choose(snap, query, "REFILL")
        local dry    = ground.choose(snap, query, "DRY")
        local team   = party.members(snap)
        if #team > 0 then
            local members = { snap }
            for _, member in ipairs(team) do
                table.insert(members, member)
            end
            refill = ground.party(members, query, "REFILL") or refill
            dry    = ground.party(members, query, "DRY") or dry
        end
        if refill == nil or refill.drain <= 0 then
            return nil
        end
        return {
            gain = refill.rate - (dry ~= nil and dry.rate or 0),
            base = dry ~= nil and dry.rate or 0,
            usage = refill.drain,
            loot = refill.loot,
            level = ground.horizon(snap, refill.rate)
        }
    end
    local best = ground.choose(snap, query)
    if best == nil or best.need <= 0 then
        return nil
    end
    local team  = party.members(snap)
    local aid   = #team > 0 and ground.crew(team, query).heal or 0
    local rates = recovery.rates(snap)
    local mend  = recovery.heal(snap)
    local own   = mend ~= nil and mend.mp > 0 and mend.hp * math.max(0, rates.mp - best.drain) / mend.mp or rates.hp
    if own + aid >= best.wear then
        return nil
    end
    local last  = best.floors[#best.floors]
    local floor = math.max(best.need * (1 + (last ~= nil and last.crowd or 0)), best.peak or 0)
    return { crisis = floor, stock = math.max(floor, best.wear * ground.horizon(snap, best.rate)), loot = best.loot }
end

local function demand(snap, query, kind)
    if kind == "mp" and (recovery.least(snap) == nil or (recovery.rates(snap).mp > 0 and recovery.spare(snap) <= 0)) then
        return nil
    end
    local base = table.concat({ score.signature(snap, ""), snap.world.map }, "|")
    local team = {}
    for _, member in ipairs(party.members(snap)) do
        table.insert(team, score.signature(member, ""))
    end
    local key = table.concat({ score.signature(snap), kind, table.concat(team, ";") }, "|")
    return memo.get(needs, snap.self.name, base, key, function()
        return evaluate(snap, query, kind) or false
    end) or nil
end

local function plan(snap, query, kind)
    local found = offers(snap, kind)
    local margin = kind == "mp" and 2 or 0
    if #found == 0 or snap.bag.capacity - snap.bag.used <= margin then
        return nil
    end
    local need = demand(snap, query, kind)
    if data.journal then
        log("debug",
            string.format("[brain:need] bot=%s map=%s kind=%s hp=%s mp=%s gain=%s base=%s crisis=%s stock=%s",
                snap.self.name or "",
                tostring(snap.world.map), kind, tostring(snap.self.hp), tostring(snap.self.mp),
                tostring(need and need.gain),
                tostring(need and need.base), tostring(need and need.crisis), tostring(need and need.stock)))
    end
    if need == nil or (need.gain ~= nil and need.gain <= 0) then
        return nil
    end
    local route = nearest(snap, query, kind, found)
    if data.journal and route == nil then
        log("debug",
            string.format("[brain:supplyend] bot=%s map=%s kind=%s reason=noroute", snap.self.name or "",
                tostring(snap.world.map), kind))
    end
    if route == nil then
        return nil
    end
    local free   = snap.bag.capacity - snap.bag.used - margin
    local money  = snap.self.money or 0
    local step   = snap.pace.step or data.walk
    local trip   = route.cost * step
    local held   = carried(snap, kind)
    local best   = nil
    local lesson = learn.pending(snap, query)
    for _, offer in ipairs(found) do
        if sold(query, offer, route.dest) then
            local amount = 0
            local length = 0
            if need.crisis ~= nil then
                amount = math.max(0, (need.stock or need.crisis) - held)
                length = math.huge
            else
                local slots = need.loot + need.usage / offer.slot
                local bag   = slots > 0 and free / slots or 0
                length      = math.min(bag, need.level or math.huge)
                amount      = math.max(0, need.usage * length - held)
            end
            local owned = 0
            for _, item in ipairs(snap.bag.items) do
                if item.name == offer.name then
                    owned = owned + item.count
                end
            end
            local budget = money
            if lesson ~= nil and (lesson.net > danger.loss(snap, query, value.baseline(snap, query)) or
                    held + (money - lesson.price) / offer.price * offer.each >= (need.crisis or 0)) then
                budget = money - lesson.price
            end
            local count = math.min(math.ceil(amount / offer.each),
                offer.capacity > 1 and offer.capacity - owned or offer.capacity,
                math.floor(budget / offer.price), free * offer.capacity)
            if count > 0 and (best == nil or offer.unit < best.unit) then
                best = { offer = offer, count = count, length = length, amount = amount, unit = offer.unit }
            end
        end
    end
    if data.journal and best == nil then
        log("debug",
            string.format(
                "[brain:supplyend] bot=%s map=%s kind=%s reason=nooffer dest=%s free=%s money=%s held=%.0f usage=%s loot=%s level=%s",
                snap.self.name or "", tostring(snap.world.map), kind, tostring(route.dest), tostring(free),
                tostring(money), held,
                tostring(need.usage), tostring(need.loot), tostring(need.level)))
    end
    if best == nil then
        return nil
    end
    local worth = value.baseline(snap, query)
    local spend = best.count * best.offer.price * worth.exchange
    local path  = value.detour(snap, query, { maps = { route.dest } }, worth)
    local risk  = path ~= nil and path.risk or 0
    local spent = (path ~= nil and path.time or trip * 2 / 1000) * (1 + #party.members(snap))
    local delta = 0
    if need.gain ~= nil then
        local shortage = best.amount / math.max(need.usage * best.length, 1)
        local cover    = held / math.max(need.usage, 1)
        local urgency  = spent > 0 and math.max(0, math.min(1, 2 - cover / spent)) or 1
        delta          = need.gain * best.length * math.min(1, shortage) * urgency - need.base * spent - spend - risk
    elseif kind == "mp" and recovery.spells(snap).charge ~= nil and (snap.self.mp or 0) + held < recovery.spells(snap).charge.mp then
        local dry   = ground.choose(snap, query, "DRY")
        local base  = dry ~= nil and dry.rate or 0
        local short = math.max(0, recovery.spells(snap).charge.mp - (snap.self.mp or 0) - held)
        local wait  = short / math.max(data.natural.mp * (snap.self.max_mp or 0), 1)
        delta       = math.max(0, worth.rate - base) * math.max(0, wait - spent) - base * spent - spend - risk
    else
        local loss   = danger.loss(snap, query, worth)
        local target = route.dest == snap.world.map and (need.stock or need.crisis) or need.crisis
        local onhand = route.dest == snap.world.map and held or held + (kind == "mp" and (snap.self.mp or 0) or 0)
        delta        = loss * math.max(0, 1 - onhand / math.max(target, 1)) - worth.value * spent - spend - risk
    end
    if data.journal then
        log("debug",
            string.format(
                "[brain:supply] bot=%s t=%s map=%s kind=%s dest=%s item=%s count=%s delta=%.0f held=%.0f spent=%.1f gain=%s base=%s crisis=%s stock=%s hp=%s mp=%s",
                snap.self.name or "", tostring(snap.now), tostring(snap.world.map), kind, tostring(route.dest),
                best.offer.name, tostring(best.count), delta, held, spent,
                tostring(need.gain), tostring(need.base), tostring(need.crisis), tostring(need.stock),
                tostring(snap.self.hp), tostring(snap.self.mp)))
    end
    if delta <= 0 then
        return nil
    end
    return { choice = best.offer, count = best.count, delta = delta }
end

local function order(snap, query)
    for _, kind in ipairs({ "mp", "hp" }) do
        local found = plan(snap, query, kind)
        if found ~= nil then
            return found.choice, found.count, found.delta
        end
    end
    return nil
end

M.order = order

function M.wanted(snap, query)
    if snap.self.ghost then
        return false
    end
    local choice = order(snap, query)
    return choice ~= nil and #query.npcs(choice.npcs) > 0
end

function M.value(snap, query)
    local _, _, delta = order(snap, query)
    return delta or 0
end

function M.propose(snap, query)
    local choice, amount = order(snap, query)
    local spot, move     = near.approach(snap, query, choice.npcs)
    if spot ~= nil then
        return { kind = ACTION.SAY, text = string.format("%s %d개 줘", choice.name, amount), gain = choice.name }
    end
    return move or { kind = ACTION.HOLD }
end

return M
