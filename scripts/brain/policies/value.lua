local buyers   = require("brain.data.buyers")
local data     = require("brain.data.combat")
local hunting  = require("brain.data.hunting")
local magic    = require("brain.data.spells")
local teleport = require("brain.data.teleport")
local danger   = require("brain.policies.danger")
local ground   = require("brain.policies.ground")

local M = {}

local function visible(snap, query)
    local found = {}
    for _, mob in ipairs(snap.world.mobs) do
        if mob.attack == "AGGRESSIVE" and mob.x ~= nil and danger.winnable(snap, query, mob) == false then
            table.insert(found, mob)
        end
    end
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

function M.baseline(snap, query)
    local hunt = ground.choose(snap, query)
    if hunt == nil or hunt.rate <= 0 then
        return { value = 0, exchange = exchange(snap, query), rate = 0, income = 0 }
    end
    local income = earnings(hunt.yields)
    local rate   = income > 0 and hunt.rate / income or exchange(snap, query)
    return { value = hunt.rate + rate * income, exchange = rate, rate = hunt.rate, income = income, hunt = hunt }
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

function M.cost(snap, query, action, worth, duration)
    if action == nil then
        return 0
    elseif action.kind == ACTION.MOVE and action.dest ~= nil then
        local trip = M.trip(snap, query, action.dest, worth)
        return trip ~= nil and trip.cost or math.huge
    end
    local time = duration or 0
    return time / 1000 * worth.value + M.stay(snap, query, time, worth)
end

local function segments(snap, route, target)
    local found = {}
    local map   = snap.world.map
    local x, y  = snap.world.x, snap.world.y
    for _, hop in ipairs(route.path or {}) do
        table.insert(found, { map = map, steps = math.abs(hop.x - x) + math.abs(hop.y - y) })
        map, x, y = hop.dest, hop.ax, hop.ay
    end
    local tx = target ~= nil and target.x or route.x or x
    local ty = target ~= nil and target.y or route.y or y
    table.insert(found, { map = map, steps = math.abs(tx - x) + math.abs(ty - y) })
    return found
end

local function threats(snap, query, map)
    local found = {}
    if map == snap.world.map then
        for _, mob in ipairs(visible(snap, query)) do
            table.insert(found, { mob = mob, weight = 1 })
        end
    else
        for _, id in ipairs(danger.perilous(snap, query)) do
            if query.native(map, id) then
                table.insert(found, { mob = hunting.mobs[id], weight = 1 })
            end
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

local function plans(snap, query, dest)
    local shunned = danger.shunned(snap, query)
    local key     = table.concat({ snap.world.map, tostring(dest.map), table.concat(dest.maps or {}, ","),
        table.concat(dest.names or {}, ","), table.concat(shunned, ","), #danger.perilous(snap, query) }, "|")
    if routes[key] ~= nil then
        return routes[key]
    end
    local where = { map = dest.map, maps = dest.maps, names = dest.names }
    local found = {
        { route = query.path(where, danger.perilous(snap, query), shunned), risky = false },
        { route = query.path(where), risky = true },
    }
    if found[1].route ~= nil or found[2].route ~= nil then
        routes[key] = found
    end
    return found
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
            return true
        end
    end
    return false
end

local function leap(snap, dest)
    local known = {}
    for _, learned in ipairs(snap.spells or {}) do
        known[learned.name] = true
    end
    for _, spell in ipairs(teleport.spells) do
        local ready = known[spell.name] and (spell.ghost == true) == snap.self.ghost and
            (snap.self.mp or 0) >= (spell.mp or 0)
        if ready and lands(snap, dest, spell.returns and teleport.returns[snap.self.nation] or spell.to) then
            return true
        end
    end
    if snap.self.ghost then
        return false
    end
    local held = {}
    for _, item in ipairs(snap.bag.items) do
        held[item.name] = true
    end
    for _, item in ipairs(teleport.items) do
        if held[item.name] and lands(snap, dest, item.returns and teleport.returns[snap.self.nation] or item.to) then
            return true
        end
    end
    return false
end

function M.trip(snap, query, dest, worth)
    local target = dest.x ~= nil and dest or nil
    local loss   = danger.loss(snap, query, worth)
    local step   = snap.pace.step or data.walk
    local best   = nil
    if target == nil and leap(snap, dest) then
        local time = 1 / magic.cast_rate
        best = {
            cost = time * worth.value + M.stay(snap, query, time * 1000, worth),
            time = time,
            risk = 0,
            decay = 0,
            period = 0,
            risky = false
        }
    end
    for _, plan in ipairs(plans(snap, query, dest)) do
        if plan.route ~= nil then
            local parts = segments(snap, plan.route, target)
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
