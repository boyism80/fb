local buyers   = require("brain.data.buyers")
local data     = require("brain.data.combat")
local hunting  = require("brain.data.hunting")
local items    = require("brain.data.items")
local shops    = require("brain.data.shops")
local teleport = require("brain.data.teleport")
local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local failure  = require("brain.policies.failure")
local ground   = require("brain.policies.ground")
local memo     = require("brain.policies.memo")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")

local M = {}

local sighted = setmetatable({}, { __mode = "k" })

local function visible(snap, query)
    if sighted[snap] ~= nil then
        return sighted[snap]
    end
    local found = {}
    for _, mob in ipairs(snap.world.mobs) do
        if mob.attack == "AGGRESSIVE" and mob.x ~= nil and danger.winnable(snap, query, mob) == false then
            table.insert(found, mob)
        end
    end
    sighted[snap] = found
    return found
end

local function earnings(yields)
    local income = 0
    for name, entry in pairs(yields or {}) do
        income = income + (buyers[name] ~= nil and buyers[name].price or 0) * entry.rate
    end
    return income
end

local function exchange(snap, query)
    local best = nil
    for _, entry in ipairs(ground.ranked(snap, query)) do
        for _, floor in ipairs(entry.candidate.floors) do
            local income = earnings(floor.yields)
            if income > 0 and floor.rate > 0 and (best == nil or floor.rate / income < best) then
                best = floor.rate / income
            end
        end
    end
    return best or 0
end

local baselines = setmetatable({}, { __mode = "k" })

function M.baseline(snap, query)
    if baselines[snap] ~= nil then
        return baselines[snap]
    end
    local hunt   = party.plan(snap, query)
    local result = nil
    if hunt == nil or hunt.rate <= 0 then
        result = { value = 0, exchange = exchange(snap, query), rate = 0, income = 0 }
    else
        local income  = earnings(hunt.yields)
        local rate    = exchange(snap, query)
        local horizon = ground.horizon(snap, hunt.rate)
        if rate <= 0 and horizon < math.huge then
            rate = hunt.rate * horizon / math.max(snap.self.money or 0, 1)
        end
        local liking = (snap.persona or {}).hunt or 1
        result       = {
            value = (hunt.rate + rate * income) * liking,
            exchange = rate,
            rate = hunt.rate,
            income = income,
            hunt = hunt
        }
    end
    baselines[snap] = result
    return result
end

local sinks = setmetatable({}, { __mode = "k" })

local function sunk(snap, mob)
    local known = sinks[snap]
    if known == nil then
        known = {}
        sinks[snap] = known
    end
    local slot = mob ~= nil and (mob.oid or mob.name) or false
    if known[slot] ~= nil then
        return known[slot] or nil
    end
    local sink = nil
    if mob ~= nil and (mob.hp or 0) > 0 then
        for _, choice in ipairs(combat.options(snap, mob).strike) do
            if (choice.mp or 0) > 0 and (choice.damage or 0) > 0 then
                local each = choice.damage / choice.mp * combat.gain(mob) / mob.hp
                sink = math.max(sink or each, each)
            end
        end
    end
    known[slot] = sink or false
    return sink
end

local errands = {}

local function errand(snap, query, name, npcs)
    return memo.get(errands, snap.self.name, tostring(snap.world.map), name, function()
        local maps = {}
        for _, spot in ipairs(query.npcs(npcs)) do
            table.insert(maps, spot.map)
        end
        local route = #maps > 0 and query.route(maps, danger.perilous(snap, query), danger.shunned(snap, query)) or nil
        return route ~= nil and { seconds = 2 * route.cost * (snap.pace.step or data.walk) / 1000 } or
        { seconds = math.huge }
    end).seconds
end

local refills = setmetatable({}, { __mode = "k" })

local function refilled(snap, query)
    if refills[snap] ~= nil then
        return refills[snap]
    end
    local worth = M.baseline(snap, query)
    local regen = recovery.rates(snap).mp + data.natural.mp * (snap.self.max_mp or 0)
    local price = regen > 0 and worth.value / regen or math.huge
    local crew  = 1 + #party.members(snap)
    for name, shop in pairs(shops) do
        local effect   = items[name]
        local restored = effect ~= nil and recovery.restored(snap, effect, "mp") or 0
        if restored > 0 then
            local uses  = math.max(shop.uses or 1, 1)
            local stack = restored * uses * math.max(shop.capacity or 1, 1)
            price       = math.min(price, shop.price / (restored * uses) * worth.exchange +
                combat.cast(snap) / 1000 * worth.value / restored +
                errand(snap, query, name, shop.npcs) * crew * worth.value / stack)
        end
    end
    refills[snap] = price
    return price
end

function M.spend(snap, query, mob)
    local found = math.min(refilled(snap, query), sunk(snap, mob) or math.huge)
    return found < math.huge and found or 0
end

function M.sip(snap, query, mob)
    local found = M.spend(snap, query, mob)
    local most  = 0
    for _, item in ipairs(combat.buffs(snap).charge == nil and snap.bag.items or {}) do
        local effect = items[item.name]
        if effect ~= nil then
            most = math.max(most, recovery.restored(snap, effect, "mp"))
        end
    end
    if most <= 0 then
        return found
    end
    return math.max(found, combat.cast(snap) / 1000 * M.baseline(snap, query).value / most)
end

function M.restock(snap, query, mob, restored)
    local price = M.spend(snap, query, mob)
    if restored <= 0 then
        return price
    end
    local mp       = snap.self.mp or 0
    local charging = combat.buffs(snap).charge
    local floor    = 0
    local sink     = price
    if sunk(snap, mob) ~= nil then
        floor = recovery.opening(snap) or 0
        sink  = sunk(snap, mob)
    elseif mob ~= nil and (mob.hp or 0) > 0 then
        for _, choice in ipairs(combat.options(snap, mob).offense) do
            if (choice.mp or 0) > 0 and (choice.damage or 0) > 0 then
                local each = choice.damage / choice.mp * combat.gain(mob) / mob.hp
                if each > price and each > sink then
                    sink  = each
                    floor = choice.mp
                end
            end
        end
    end
    if charging ~= nil and charging.mp > 0 and mp < charging.mp then
        local unlocked = charging.chance * ((snap.self.max_mp or 0) - mp) / charging.mp * price
        if unlocked > sink then
            floor = charging.mp
            sink  = unlocked
        end
    end
    return math.min(restored, math.max(0, floor - mp)) * sink / restored
end

local powers = {}

function M.power(snap, query)
    return memo.get(powers, snap.self.name, "", score.signature(snap, ""), function()
        local ability = score.capability(score.rested(snap), query)
        return ability.dps.single * ability.ehp
    end)
end

local plains = setmetatable({}, { __mode = "k" })

function M.refill(snap, query, worth)
    if plains[snap] ~= nil then
        return plains[snap]
    end
    local regen = recovery.rates(snap).mp + data.natural.mp * (snap.self.max_mp or 0)
    local price = regen > 0 and worth.value / regen or math.huge
    for name, shop in pairs(shops) do
        local effect   = items[name]
        local restored = effect ~= nil and recovery.restored(snap, effect, "mp") or 0
        if restored > 0 then
            price = math.min(price, shop.price / restored * worth.exchange)
        end
    end
    plains[snap] = price < math.huge and price or 0
    return plains[snap]
end

local manas = {}

function M.mana(snap, query)
    local period = (snap.now or 0) // data.rerank
    local known  = manas[snap.self.name]
    if known ~= nil and known.period == period then
        return known.found
    end
    local full = ground.choose(snap, query, "REFILL")
    local dry  = ground.choose(snap, query, "DRY")
    local team = party.members(snap)
    if #team > 0 then
        local members = { snap }
        for _, member in ipairs(team) do
            table.insert(members, member)
        end
        full = ground.party(members, query, "REFILL") or full
        dry  = ground.party(members, query, "DRY") or dry
    end
    local found = 0
    if full ~= nil and full.drain > 0 then
        found = math.max(0, (full.rate - (dry ~= nil and dry.rate or 0)) / full.drain)
    end
    manas[snap.self.name] = { period = period, found = found }
    return found
end

function M.deserted(snap, query, action)
    local worth = M.baseline(snap, query)
    local cost  = 0
    local away  = nil
    for _, member in ipairs(party.members(snap)) do
        if member.world.map == snap.world.map and member.world.x ~= nil and member.self.ghost == false then
            local hits = 0
            for _, mob in ipairs(snap.world.mobs or {}) do
                if mob.x ~= nil and combat.hits(mob) and (mob.speed or 0) > 0 and
                    math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) == 1 then
                    hits = hits + combat.blow(member, mob) / mob.speed * 1000
                end
            end
            if hits > 0 then
                if away == nil then
                    local trip = action ~= nil and action.dest ~= nil and M.trip(snap, query, action.dest, worth) or nil
                    away = trip ~= nil and 2 * trip.time or combat.cast(snap) / 1000
                end
                cost = cost +
                math.min(1, hits * away / math.max(member.self.hp or 1, 1)) * danger.loss(member, query, worth)
            end
        end
    end
    return cost
end

function M.stay(snap, query, duration, worth)
    local step    = snap.pace.step or data.walk
    local chances = {}
    for _, mob in ipairs(visible(snap, query)) do
        local chases = mob.speed ~= nil and mob.speed <= step
        local close  = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= data.reach
        table.insert(chances, danger.chance(snap, query, mob, (chases or close) and duration or 0))
    end
    return danger.combine(chances) * danger.loss(snap, query, worth)
end

function M.peril(snap, query, point)
    local step    = snap.pace.step or data.walk
    local dwell   = combat.cast(snap)
    local range   = math.max(data.sight.x, data.sight.y)
    local chances = {}
    local seen    = {}
    local function add(mob, x, y, weight)
        local dx = math.abs(x - point.x)
        local dy = math.abs(y - point.y)
        if (mob.speed or 0) > 0 and dx <= data.sight.x and dy <= data.sight.y then
            local distance = dx + dy
            local inside   = math.max(0, range - distance) * step + dwell
            local approach = math.max(0, distance - data.reach) * mob.speed
            table.insert(chances, weight * danger.chance(snap, query, mob, inside - approach))
        end
    end
    if point.map == snap.world.map then
        for _, mob in ipairs(visible(snap, query)) do
            seen[mob.id or mob.name] = true
            add(mob, mob.x, mob.y, 1)
        end
    end
    local perils = snap.record.memory.perils or {}
    local peril  = perils[tostring(point.map)] or perils[point.map]
    if type(peril) == "table" and type(peril.stats) == "table" and peril.stats.x ~= nil and
        seen[peril.mob or peril.stats.name] == nil then
        add(peril.stats, peril.stats.x, peril.stats.y, danger.weight(snap, peril))
    end
    return danger.combine(chances)
end

function M.cost(snap, query, action, worth, duration)
    if action == nil then
        return 0
    elseif action.kind == ACTION.MOVE and action.dest ~= nil then
        local trip = M.detour(snap, query, action.dest, worth)
        return trip ~= nil and trip.cost or math.huge
    end
    local time = duration or 0
    return time / 1000 * worth.value + M.stay(snap, query, time, worth)
end

local function segments(origin, route, target)
    local found = {}
    local map   = origin.map
    local x, y  = origin.x, origin.y
    for _, hop in ipairs(route.path or {}) do
        table.insert(found, { map = map, steps = math.abs(hop.x - x) + math.abs(hop.y - y) })
        map, x, y = hop.dest, hop.ax, hop.ay
    end
    local tx = target ~= nil and target.x or route.x or x
    local ty = target ~= nil and target.y or route.y or y
    table.insert(found, { map = map, steps = math.abs(tx - x) + math.abs(ty - y) })
    return found
end

local nativity = {}

local function natives(perilous, query, map)
    local key = table.concat(perilous, ",")
    if nativity.key ~= key then
        nativity = { key = key, maps = {} }
    end
    local known = nativity.maps[map]
    if known == nil then
        known = {}
        for _, id in ipairs(perilous) do
            if query.native(map, id) then
                table.insert(known, { mob = hunting.mobs[id], weight = 1 })
            end
        end
        nativity.maps[map] = known
    end
    return known
end

local function threats(snap, query, map)
    local found = {}
    if map == snap.world.map then
        for _, mob in ipairs(visible(snap, query)) do
            table.insert(found, { mob = mob, weight = 1 })
        end
    else
        for _, entry in ipairs(natives(danger.perilous(snap, query), query, map)) do
            table.insert(found, entry)
        end
    end
    local seen = (snap.record.memory.perils or {})[tostring(map)] or (snap.record.memory.perils or {})[map]
    if type(seen) == "table" then
        local mob = hunting.mobs[seen.mob] or seen.stats
        if mob ~= nil then
            table.insert(found, { mob = mob, weight = danger.weight(snap, seen), period = danger.period(seen) })
        end
    end
    return found
end

function M.exposure(snap, query, parts)
    local step   = snap.pace.step or data.walk
    local all    = {}
    local native = {}
    local period = 0
    for _, part in ipairs(parts) do
        for _, threat in ipairs(threats(snap, query, part.map)) do
            local chases = threat.mob.speed ~= nil and threat.mob.speed <= step
            local expose = chases and part.steps * step or 0
            local chance = threat.weight * danger.chance(snap, query, threat.mob, expose)
            table.insert(all, chance)
            if threat.period == nil then
                table.insert(native, chance)
            elseif chance > 0 then
                period = math.max(period, threat.period)
            end
        end
    end
    return danger.combine(all), danger.combine(native), period
end

local routes = {}

local function plans(snap, query, dest, origin)
    local shunned  = danger.shunned(snap, query)
    local perilous = party.perilous(snap, query)
    local base     = table.concat({ snap.world.map, table.concat(shunned, ","), #perilous }, "|")
    local key      = table.concat({ origin ~= nil and origin.map or "", tostring(dest.map),
        table.concat(dest.maps or {}, ","), table.concat(dest.names or {}, ",") }, "|")
    local where    = { map = dest.map, maps = dest.maps, names = dest.names }
    local found    = memo.get(routes, snap.self.name, base, key, function()
        local found = {
            { route = query.path(where, perilous, shunned, origin), risky = false },
            { route = query.path(where, nil, nil, origin), risky = true },
        }
        return (found[1].route ~= nil or found[2].route ~= nil) and found or false
    end)
    return found or { { risky = false }, { risky = true } }
end

local function lands(snap, dest, targets)
    if targets == nil or #targets == 0 then
        return false
    end
    local wanted = {}
    for _, map in ipairs(dest.maps or {}) do
        wanted[map] = true
    end
    for _, name in ipairs(dest.names or {}) do
        wanted[name] = true
    end
    if dest.map ~= nil then
        wanted[dest.map] = true
    end
    for _, target in ipairs(targets) do
        if wanted[target] ~= nil then
            return target
        end
    end
    return nil
end

function M.leap(snap, dest)
    local known = {}
    for _, learned in ipairs(snap.spells or {}) do
        known[learned.name] = true
    end
    for _, spell in ipairs(teleport.spells) do
        local ready = known[spell.name] and (spell.ghost == true) == snap.self.ghost and
            (snap.self.mp or 0) >= (spell.mp or 0)
        local land  = ready and lands(snap, dest, spell.returns and teleport.returns[snap.self.nation] or spell.to) or
            nil
        if land ~= nil then
            return { spell = spell, land = land }
        end
    end
    if snap.self.ghost then
        return nil
    end
    local held = {}
    for _, item in ipairs(snap.bag.items) do
        held[item.name] = true
    end
    for _, item in ipairs(teleport.items) do
        local land = held[item.name] and
            lands(snap, dest, item.returns and teleport.returns[snap.self.nation] or item.to) or nil
        if land ~= nil then
            return { item = item, land = land }
        end
    end
    return false
end

local function walk(snap, query, dest, worth, origin, best)
    local target = dest.x ~= nil and dest or nil
    local loss   = danger.loss(snap, query, worth)
    local step   = snap.pace.step or data.walk
    local start  = origin or { map = snap.world.map, x = snap.world.x, y = snap.world.y }
    for _, plan in ipairs(plans(snap, query, dest, origin)) do
        if plan.route ~= nil then
            local parts = segments(start, plan.route, target)
            local steps = 0
            for _, part in ipairs(parts) do
                steps = steps + part.steps
            end
            local time                  = math.max(plan.route.cost, steps) * step / 1000
            local chance, fixed, period = M.exposure(snap, query, parts)
            local risk                  = chance * loss
            local cost                  = time * worth.value + risk
            if best == nil or cost < best.cost then
                best = {
                    cost = cost,
                    time = time,
                    risk = risk,
                    decay = risk - fixed * loss,
                    period = period,
                    risky = plan.risky,
                    route = plan.route
                }
            end
        end
    end
    return best
end

function M.trip(snap, query, dest, worth)
    local best = nil
    local jump = dest.x == nil and M.leap(snap, dest) or nil
    if jump ~= nil then
        local time = (combat.cast(snap) + data.transfer) / 1000
        best = {
            route = type(jump.land) == "number" and { dest = jump.land } or nil,
            cost = time * worth.value + M.stay(snap, query, time * 1000, worth),
            time = time,
            risk = 0,
            decay = 0,
            period = 0,
            risky = false
        }
    end
    return walk(snap, query, dest, worth, nil, best)
end

function M.objective(snap, query)
    local order = snap.record.memory.order
    if order ~= nil and snap.self.ghost == false and failure.remaining(snap, "ORDER") == 0 then
        if order.task == "GO" then
            return { task = "ORDER", dest = order.dest }
        elseif order.task == "HUNT" then
            local maps = {}
            for _, habitat in ipairs(query.habitats(order.names or {})) do
                table.insert(maps, habitat.id)
            end
            return { task = "ORDER", dest = #maps > 0 and { maps = maps } or nil }
        end
    end
    local hunt = party.plan(snap, query)
    return { task = "FIELD", dest = hunt ~= nil and { maps = { hunt.floors[1].map } } or nil }
end

function M.detour(snap, query, dest, worth)
    local trip = M.trip(snap, query, dest, worth)
    local goal = M.objective(snap, query).dest
    if trip == nil or goal == nil or (trip.route == nil and dest.x == nil) then
        return trip
    end
    local route   = trip.route or {}
    local spot    = dest.spots ~= nil and dest.spots[route.dest] or nil
    local arrival = {
        map = route.dest or dest.map or snap.world.map,
        x = dest.x or (spot ~= nil and spot.x) or route.x or snap.world.x,
        y = dest.y or (spot ~= nil and spot.y) or route.y or snap.world.y
    }
    local inside  = 0
    if spot ~= nil and route.x ~= nil and route.y ~= nil then
        local range = math.min(data.sight.x, data.sight.y)
        local steps = math.abs(spot.x - route.x) + math.abs(spot.y - route.y)
        inside      = math.max(0, steps - range) * (snap.pace.step or data.walk) / 1000
    end
    local onward = walk(snap, query, goal, worth, arrival)
    local direct = walk(snap, query, goal, worth)
    if onward == nil or direct == nil then
        return trip
    end
    local time = math.max(0, trip.time + inside + onward.time - direct.time)
    local risk = math.max(0, trip.risk + onward.risk - direct.risk)
    return {
        cost = time * worth.value + risk,
        time = time,
        risk = risk,
        decay = trip.decay,
        period = trip.period,
        risky = trip.risky,
        route = trip.route
    }
end

function M.patience(trip, worth)
    if trip == nil or trip.decay <= 0 or trip.period <= 0 then
        return 0
    end
    local period = trip.period / 1000
    if worth.value <= 0 then
        return period
    end
    return math.max(0, period * math.log(trip.decay * math.log(2) / (worth.value * period), 2))
end

return M
