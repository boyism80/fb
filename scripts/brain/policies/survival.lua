local data    = require("brain.data.combat")
local hunting = require("brain.data.hunting")
local combat  = require("brain.policies.combat")

local M = {}

local typical = nil

function M.respawn(mob)
    if mob ~= nil and (mob.rezen or 0) > 0 then
        return mob.rezen
    end
    if typical == nil then
        local values = {}
        for _, known in pairs(hunting.mobs) do
            if (known.rezen or 0) > 0 then
                table.insert(values, known.rezen)
            end
        end
        table.sort(values)
        typical = values[(#values + 1) // 2] or 0
    end
    return typical
end

local function pressure(snap, mob)
    if combat.hits(mob) == false or mob.speed == nil or mob.speed <= 0 then
        return 0
    end
    return combat.blow(snap, mob) / mob.speed
end

function M.crowd(snap, entries, time, fighters)
    local arrival = 0
    local present = 0
    local weight  = 0
    local burden  = 0
    local worst   = nil
    for _, entry in ipairs(entries) do
        local mob = entry.mob
        if mob.attack == "AGGRESSIVE" and combat.hits(mob) then
            local period  = M.respawn(mob)
            arrival       = arrival + (period > 0 and entry.count / period or 0)
            present       = present + entry.count
            weight        = weight + entry.count
            burden        = burden + entry.count * pressure(snap, mob)
            local _, high = combat.spread(snap, mob)
            if worst == nil or high > worst.high then
                worst = { mob = mob, high = high }
            end
        end
    end
    local result = { states = { 1 }, extra = 0, pressure = 0, worst = worst }
    if present == 0 or time == nil or time <= 0 or time == math.huge then
        return result
    end
    local load     = arrival * time / math.max(fighters or 1, 1)
    local capacity = math.min(present, 4 * math.max(fighters or 1, 1))
    local states   = {}
    local total    = 0
    for n = 0, capacity do
        states[n + 1] = load ^ n
        total         = total + states[n + 1]
    end
    local extra = 0
    for n = 0, capacity do
        states[n + 1] = states[n + 1] / total
        extra         = extra + states[n + 1] * math.max(n - 1, 0)
    end
    result.states   = states
    result.load     = load
    result.extra    = extra
    result.pressure = extra * burden / math.max(weight, 1)
    return result
end

function M.ending(snap, result, crowd)
    if result.time == math.huge then
        return -math.huge
    end
    return snap.self.hp + result.healed - result.taken - crowd.pressure * result.time
end

function M.burst(snap, crowd, time, hp)
    local worst = crowd.worst
    if worst == nil or worst.mob.speed == nil or worst.mob.speed <= 0 or time <= 0 then
        return 0
    end
    local low, high = combat.spread(snap, worst.mob)
    local round     = 0
    for index, chance in ipairs(crowd.states) do
        round = round + chance * combat.reaches(math.max(index - 1, 1), low, high, hp)
    end
    return 1 - (1 - round) ^ (time / worst.mob.speed)
end

function M.contact(snap, mob, time)
    local burden = 0
    local extra  = 0
    for _, seen in ipairs(snap.world.mobs or {}) do
        if seen.oid ~= mob.oid and seen.x ~= nil and seen.attack == "AGGRESSIVE" and combat.hits(seen) and
            (seen.speed or 0) > 0 then
            local distance = math.abs(seen.x - snap.world.x) + math.abs(seen.y - snap.world.y)
            local reached  = math.max(0, distance - data.reach) * seen.speed
            local share    = time == math.huge and 1 or time > 0 and math.max(0, time - reached) / time or 0
            extra          = extra + share
            burden         = burden + share * pressure(snap, seen)
        end
    end
    return { states = { 1 }, extra = extra, pressure = burden }
end

return M
