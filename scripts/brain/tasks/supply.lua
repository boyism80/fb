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

local M = {}

local trips = {}
local needs = {}

local function carried(snap, kind)
    local total = 0
    for _, dose in ipairs(recovery.doses(snap, kind)) do
        total = total + dose.amount * dose.count
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
        if recovery.least(snap) == nil or recovery.rates(snap).mp > 0 then
            return nil
        end
        local refill = ground.choose(snap, query, "REFILL")
        local dry    = ground.choose(snap, query, "DRY")
        local team   = party.members(snap)
        if #team > 0 then
            local members = { snap }
            for _, member in ipairs(team) do
                table.insert(members, member)
            end
            local full, front = ground.party(members, query, "REFILL")
            if front == 1 then
                refill = full
                dry    = (ground.party(members, query, "DRY"))
            end
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
    if recovery.heal(snap) ~= nil then
        return nil
    end
    local best = ground.choose(snap, query)
    if best == nil or best.need <= 0 then
        return nil
    end
    return { crisis = best.need, loot = best.loot }
end

local function demand(snap, query, kind)
    if (kind == "mp" and (recovery.least(snap) == nil or recovery.rates(snap).mp > 0)) or
        (kind == "hp" and recovery.heal(snap) ~= nil) then
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
    if #found == 0 or snap.bag.capacity - snap.bag.used <= 1 then
        return nil
    end
    local need = demand(snap, query, kind)
    if need == nil or (need.gain ~= nil and need.gain <= 0) then
        return nil
    end
    local route = nearest(snap, query, kind, found)
    if route == nil then
        return nil
    end
    local free  = snap.bag.capacity - snap.bag.used - 1
    local money = snap.self.money or 0
    local step  = snap.pace.step or data.walk
    local trip  = route.cost * step
    local held  = carried(snap, kind)
    local best  = nil
    for _, offer in ipairs(found) do
        if sold(query, offer, route.dest) then
            local amount = 0
            local length = 0
            if need.crisis ~= nil then
                amount = math.max(0, need.crisis - held)
                length = math.huge
            else
                local slots = need.loot + need.usage / offer.slot
                local bag   = slots > 0 and free / slots or 0
                length      = math.min(bag, need.level)
                amount      = math.max(0, need.usage * length - held)
            end
            local count = math.min(math.ceil(amount / offer.each), offer.capacity, math.floor(money / offer.price),
                free * offer.capacity)
            if count > 0 and (best == nil or offer.unit < best.unit) then
                best = { offer = offer, count = count, length = length, amount = amount, unit = offer.unit }
            end
        end
    end
    if best == nil then
        return nil
    end
    local worth = value.baseline(snap, query)
    local spend = best.count * best.offer.price * worth.exchange
    local path  = value.detour(snap, query, { maps = { route.dest } }, worth)
    local risk  = path ~= nil and path.risk or 0
    local spent = path ~= nil and path.time or trip * 2 / 1000
    local delta = 0
    if need.gain ~= nil then
        delta = need.gain * best.length - need.base * spent - spend - risk
    else
        local loss = danger.loss(snap, query, worth)
        delta = loss * math.max(0, 1 - held / math.max(need.crisis, 1)) - worth.value * spent - spend - risk
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
