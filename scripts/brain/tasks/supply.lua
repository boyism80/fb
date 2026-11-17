local data     = require("brain.data.combat")
local items    = require("brain.data.items")
local shops    = require("brain.data.shops")
local combat   = require("brain.policies.combat")
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

local NEUTRAL = { defense = 0, ward = 0, resists = {}, size = "SMALL" }

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
                amount = amount,
                uses = shop.uses or 1,
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

-- The mana one refill is drunk for: the 공력증강 a bot casts again, which refills the rest, the least a strike that
-- spends all the mana is cast with, as more is spent on it anyway, else the dearest spell the bot casts (user 11-14).
local function refilled(snap, kind)
    if kind ~= "mp" then
        return nil
    end
    local charge = recovery.spells(snap).charge
    if charge ~= nil then
        return charge.mp
    end
    if recovery.draining(snap) then
        return recovery.opening(snap)
    end
    local options = combat.options(snap, NEUTRAL)
    local most    = 0
    for _, group in ipairs({ options.offense, options.strike, options.heal }) do
        for _, choice in ipairs(group or {}) do
            most = math.max(most, type(choice.mp) == "number" and choice.mp or 0)
        end
    end
    return most > 0 and most or nil
end

-- A refill costs the price of its drinks and the time they take past the skill cooldown the bot waits out anyway. A
-- drink takes the time the bot measures for its drinks, so the two 동동주 a rogue drinks for its 필살검무 cost no time
-- and the cheapest dose wins, while twenty for a 600 mana spell keep the bot off its fight for a second and the
-- dearer 홍주 at 300 a drink does better (user 11-14). Priced by the point alone, 동동주 always won; timed by its casts,
-- which the server runs a spell script for, a drink took longer than the cooldown and a rogue drank 홍주 and 백세주
-- 450 times in ten minutes (online trio 135922).
local function refill_cost(snap, offer, need, worth)
    local drinks = math.ceil(need / offer.amount)
    local wait   = math.max(0, drinks * ((snap.pace or {}).use or 0) - combat.interval())
    return drinks * offer.price / offer.uses * worth.exchange + wait / 1000 * worth.value
end

-- A dose the money buys the whole stock of comes before one it does not, and short of money for any, the one that
-- buys the most mana: a bot short of money buys the cheaper dose (user 11-14).
local function better(found, best)
    if best == nil then
        return true
    elseif found.covers ~= best.covers then
        return found.covers
    elseif found.covers == false and found.mana ~= best.mana then
        return found.mana > best.mana
    elseif found.cost ~= best.cost then
        return found.cost < best.cost
    end
    return found.unit < best.unit
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
    local shop     = snap.record.memory.shop
    local kept     = shop ~= nil and (snap.now or 0) - shop.at < data.rerank and shop.map or nil
    local key      = table.concat({ score.signature(snap), kind, table.concat(names, ","),
        (snap.world.x or 0) // data.sight.x, (snap.world.y or 0) // data.sight.y, tostring(kept) }, "|")
    return memo.get(trips, snap.self.name, base, key, function()
        local maps  = {}
        local route = nil
        for _, offer in ipairs(found) do
            for _, spot in ipairs(query.npcs(offer.npcs)) do
                table.insert(maps, spot.map)
                if spot.map == kept and route == nil then
                    route = query.route({ kept }, perilous, shunned)
                end
            end
        end
        route = route or (#maps > 0 and query.route(maps, perilous, shunned)) or nil
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

-- Hp supplies are not needed beside a crewmate with a heal skill, who brings its own mana supplies. The
-- group's makeup counts, not a member's state at the moment (a dead healer comes back). Buying food anyway
-- sent a warrior out mid-hunt for 229 hp items while its poet had healed it 638 times and it had eaten 8
-- (WA+PO 세작, 563s). My own heal is weighed by the ground like any supply (evaluate), as it may heal far
-- less than the ground wears: counted as enough whenever it was learned, a rogue that healed 200 hp for 100
-- mp never bought 웅담 again once its 1000 were gone, fled its grounds eight times and hopped between them
-- (offline solo seed 107, 297-717s, run 21:55).
local function healed(snap)
    for _, member in ipairs(party.members(snap)) do
        if recovery.heal(member) ~= nil then
            return true
        end
    end
    return false
end

local function evaluate(snap, query, kind)
    if kind == "mp" then
        if recovery.least(snap) == nil and recovery.upkeep(snap) <= 0 then
            return nil
        end
        local refill = ground.choose(snap, query, "REFILL")
        local dry    = ground.choose(snap, query, "DRY")
        local income = recovery.rates(snap).mp
        local drains = recovery.draining(snap)
        if drains == false and income >= (refill ~= nil and ground.drain(refill, snap) or 0) and income > 0 then
            local spare = recovery.ignition(snap)
            -- Stocked for the worst a hunt brings, the mana kept back with my hp gone: kept at the hp of the moment,
            -- what a bot bought would turn with every blow it took.
            return spare > 0 and { crisis = spare, stock = recovery.spare(snap, 0), loot = 0 } or nil
        end
        local team = party.members(snap)
        if #team > 0 then
            local members = { snap }
            for _, member in ipairs(team) do
                table.insert(members, member)
            end
            refill = ground.party(members, query, "REFILL") or refill
            dry    = ground.party(members, query, "DRY") or dry
        end
        -- A strike that spends all the mana keeps none of the income, and every strike is drunk for at its least mana:
        -- the stock is counted at a strike each dose and cast (battle.candidates) all the hunt long, more than is ever
        -- drunk. Weighed against its 운기, a rogue carried one 동동주, drank its 200 in 4 minutes, walked out of 흑해골굴
        -- with 15 mana, short of 비영사천문, and died there (online trio 100815; user 11-14).
        if drains then
            local opening = recovery.opening(snap) or 0
            return opening > 0 and {
                crisis = opening,
                usage = opening * 1000 / (2 * combat.cast(snap)),
                loot = 0,
                yields = refill ~= nil and refill.yields or nil,
                level = refill ~= nil and ground.lifetime(snap, refill.rate) or nil,
                lap = refill ~= nil and ground.lap(refill) or nil
            } or nil
        end
        if refill == nil or ground.drain(refill, snap) <= 0 then
            return nil
        end
        return {
            gain = (refill.top or refill.rate) - (dry ~= nil and (dry.top or dry.rate) or 0),
            base = dry ~= nil and dry.rate or 0,
            usage = ground.drain(refill, snap),
            loot = refill.loot,
            yields = refill.yields,
            level = ground.lifetime(snap, refill.rate),
            lap = ground.lap(refill)
        }
    end
    local best = ground.choose(snap, query)
    if best == nil or best.need <= 0 or best.wear <= 0 then
        return nil
    end
    local broke = setmetatable({
        self = setmetatable({ money = 0 }, { __index = snap.self }),
        hypothesis = (snap.hypothesis or "") .. "broke"
    }, { __index = snap })
    local bare  = ground.choose(broke, query)
    local gain  = best.rate - (bare ~= nil and bare.rate or 0)
    if gain <= 0 then
        return nil
    end
    return {
        gain = gain,
        base = bare ~= nil and bare.rate or 0,
        usage = best.wear,
        loot = best.loot,
        yields = best.yields,
        level = ground.horizon(snap, best.rate),
        lap = ground.lap(best)
    }
end

local function demand(snap, query, kind)
    if kind == "mp" and (recovery.least(snap) == nil or (recovery.rates(snap).mp > 0 and recovery.spare(snap) <= 0)) then
        return nil
    end
    if kind == "hp" and healed(snap) then
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
    if need == nil then
        return nil
    end
    local gain = need.gain
    if gain ~= nil and gain <= 0 then
        local shop = snap.record.memory.shop
        gain = shop ~= nil and shop.kind == kind and (snap.now or 0) - shop.at < data.rerank and
            shop.gain or nil
        if gain == nil then
            return nil
        end
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
    local worth  = value.baseline(snap, query)
    local refill = refilled(snap, kind)
    -- The bag holds the stock and the loot of the hunt together, and the stock is sized to run out when the loot fills
    -- the rest: sized to the whole free bag, a stock left the loot no room (user 11-14). It is bought for the whole lap
    -- the plan makes, from its first floor to its last (user 11-16), which is the same lap wherever the bot asks from
    -- (ground.lapped): the trip a bot sets out on for a lap is then still wanted at the shop. The time to the next
    -- level is no bound on it, as the lap is hunted to its end across a level.
    local litter = 0
    for _, entry in pairs(need.yields or {}) do
        litter = litter + entry.rate / math.max(entry.capacity or 1, 1)
    end
    for _, offer in ipairs(found) do
        if sold(query, offer, route.dest) then
            local amount = 0
            local length = math.huge
            if need.usage ~= nil and need.usage > 0 then
                local slots = need.usage / offer.slot + litter
                length      = math.min(slots > 0 and free / slots or 0, need.lap or need.level or math.huge)
            end
            if need.crisis ~= nil and need.usage == nil then
                amount = math.max(0, (need.stock or need.crisis) - held)
            else
                amount = math.max(0, need.usage * length - held)
            end
            local owned = 0
            for _, item in ipairs(snap.bag.items) do
                if item.name == offer.name then
                    owned = owned + item.count
                end
            end
            local budget = money
            if lesson ~= nil and (lesson.net > party.death(snap, query, snap, value.baseline(snap, query)) or
                    held + (money - lesson.price) / offer.price * offer.each >= (need.crisis or 0)) then
                budget = money - lesson.price
            end
            local count   = math.min(math.ceil(amount / offer.each),
                offer.capacity > 1 and offer.capacity - owned or offer.capacity,
                math.floor(budget / offer.price), free * offer.capacity)
            local affords = math.floor(budget / offer.price) * offer.each
            local option  = count > 0 and {
                offer = offer,
                count = count,
                amount = amount,
                unit = offer.unit,
                covers = affords >= amount,
                mana = affords,
                cost = refill ~= nil and refill_cost(snap, offer, refill, worth) or offer.unit,
            } or nil
            if option ~= nil and better(option, best) then
                local span    = need.usage ~= nil and need.usage > 0 and (held + count * offer.each) / need.usage or
                    length
                option.length = math.min(length, span)
                best          = option
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
    local spend = best.count * best.offer.price * worth.exchange
    local path  = value.detour(snap, query, { maps = { route.dest } }, worth)
    local risk  = path ~= nil and path.risk or 0
    local reach = 1 - (path ~= nil and path.chance or 0)
    local spent = (path ~= nil and path.time or trip * 2 / 1000) * (1 + #party.members(snap))
    local gone  = snap.record.memory.shop
    if gone == nil or gone.map ~= route.dest or gone.spent == nil or (snap.now or 0) - gone.at >= data.rerank then
        gone = nil
    end
    if gone ~= nil then
        spent = math.min(spent, gone.spent)
    end
    local stake = math.max(party.death(snap, query, snap, worth), gone ~= nil and gone.stake or 0)
    local delta = 0
    if need.gain ~= nil then
        local shortage = best.amount / math.max(need.usage * best.length, 1)
        local cover    = held / math.max(need.usage, 1)
        local urgency  = gone == nil and route.dest ~= snap.world.map and spent > 0 and
            math.max(0, math.min(1, 2 - cover / spent)) or 1
        delta          = gain * best.length * math.min(1, shortage) * urgency * reach - need.base * spent - spend - risk
    else
        local loss   = stake
        local stock  = need.stock or (need.usage ~= nil and held + best.amount) or need.crisis
        local target = route.dest == snap.world.map and stock or need.crisis
        local charge = recovery.spells(snap).charge
        local onhand = (charge == nil and kind == "mp" and route.dest ~= snap.world.map) and held + (snap.self.mp or 0) or
            held
        delta        = loss * math.max(0, 1 - onhand / math.max(target, 1)) * reach - worth.value * spent - spend - risk
    end
    if route.dest ~= snap.world.map then
        local chances = {}
        for _, mob in ipairs(snap.world.mobs or {}) do
            if mob.x ~= nil and combat.hits(mob) and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                table.insert(chances,
                    combat.lethal(snap, mob, math.min(spent * 1000, 2 * data.sight.x * step), snap.self.hp))
            end
        end
        delta = delta - danger.combine(chances) * party.death(snap, query, snap, worth)
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
    -- A trip is made only when it pays, servicing or not; the few items every trip needs are bought by
    -- essential. Buying whatever had a plan while servicing filled bags with supplies nobody would use.
    if delta <= 0 then
        return nil
    end
    return {
        choice = best.offer,
        count = best.count,
        delta = delta,
        dest = route.dest,
        spent = spent,
        stake = stake,
        gain =
            gain,
        kind = kind,
        detail = {
            item = best.offer.name,
            count = best.count,
            amount = best.amount,
            length = best.length,
            usage = need.usage,
            level = need.level,
            crisis = need.crisis,
            stock = need.stock,
            held = held,
            free = free,
            litter = litter,
            delta = delta
        }
    }
end

local function essential(snap, query)
    if snap.record.memory.servicing == nil then
        return nil
    end
    local known = {}
    for _, learned in ipairs(snap.spells or {}) do
        known[learned.name] = true
    end
    local wanted = { known["귀환"] == nil and "노란비서" or nil, (snap.self.max_mp or 0) > 0 and "동동주" or nil }
    for index = 1, 2 do
        local name = wanted[index]
        local shop = name ~= nil and shops[name] or nil
        if shop ~= nil then
            local owned = 0
            for _, item in ipairs(snap.bag.items) do
                if item.name == name then
                    owned = owned + item.count
                end
            end
            local budget = shop.price > 0 and (snap.self.money or 0) // shop.price or 0
            local count  = math.min(shop.capacity - owned, budget)
            if count > 0 then
                local offer = { name = name, price = shop.price, capacity = shop.capacity, npcs = shop.npcs }
                local route = nearest(snap, query, "essential" .. name, { offer })
                if route ~= nil then
                    return offer, count, 1, route.dest, {}, 0, 0, nil, "essential"
                end
            end
        end
    end
    return nil
end

local function order(snap, query)
    local choice, count, delta, dest, listed, spent, stake, gain, kind = essential(snap, query)
    if choice ~= nil then
        return choice, count, delta, dest, listed, spent, stake, gain, kind
    end
    local best   = nil
    local rivals = {}
    for _, kind in ipairs({ "mp", "hp" }) do
        local found = plan(snap, query, kind)
        if found ~= nil then
            table.insert(rivals, { key = found.choice.name, value = found.delta })
        end
        if found ~= nil and (best == nil or found.delta > best.delta) then
            best = found
        end
    end
    if best == nil then
        return nil
    end
    return best.choice, best.count, best.delta, best.dest, rivals, best.spent, best.stake, best.gain, best.kind,
        best.detail
end

M.order = order

M.need_of = demand

function M.ordered(snap, query)
    return order(snap, query) ~= nil
end

function M.sold_somewhere(snap, query)
    local choice = order(snap, query)
    return #query.npcs(choice.npcs) > 0
end

function M.at_shop(snap, query)
    local choice = order(snap, query)
    return near.approach(snap, query, choice.npcs) ~= nil
end

function M.value(snap, query)
    local _, _, delta = order(snap, query)
    return delta or 0
end

function M.buy(snap, query)
    local choice, amount, delta, _, rivals, _, _, _, _, detail = order(snap, query)
    return {
        kind = ACTION.SAY,
        text = string.format("%s %d개 줘", choice.name, amount),
        gain = choice.name,
        rivals = rivals,
        scale = delta,
        supply = detail
    }
end

function M.walk(snap, query)
    local choice, _, delta, dest, rivals, spent, stake, gain, kind, detail = order(snap, query)
    local _, move = near.approach(snap, query, choice.npcs)
    if move ~= nil and move.dest.maps ~= nil then
        move.dest.maps = { dest }
        move.shop      = { map = dest, at = snap.now or 0, spent = spent, stake = stake, gain = gain, kind = kind }
        move.rivals    = rivals
        move.scale     = delta
        move.supply    = detail
    end
    return move or { kind = ACTION.HOLD }
end

return M
