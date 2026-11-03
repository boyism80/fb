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

function M.crowd(snap, entries, time, fighters, density)
    local arrival = 0
    local present = 0
    local total   = 0
    local burden  = 0
    for _, entry in ipairs(entries) do
        local mob = entry.mob
        total     = total + entry.count
        if mob.attack == "AGGRESSIVE" and combat.hits(mob) then
            local period = M.respawn(mob)
            arrival      = arrival + (period > 0 and entry.count / period or 0)
            present      = present + entry.count
            burden       = burden + entry.count * pressure(snap, mob)
        end
    end
    local result = { extra = 0, pressure = 0 }
    if present == 0 or time == nil or time <= 0 or time == math.huge then
        return result
    end
    local tiles    = (2 * data.sight.x + 1) * (2 * data.sight.y + 1)
    local standing = (density or 0) * tiles * present / math.max(total, 1)
    local load     = (arrival * time + standing) / math.max(fighters or 1, 1)
    local capacity = math.min(present, data.adjacent * math.max(fighters or 1, 1))
    local weights  = {}
    local sum      = 0
    for n = 0, capacity do
        weights[n + 1] = load ^ n
        sum            = sum + weights[n + 1]
    end
    for n = 0, capacity do
        result.extra = result.extra + weights[n + 1] / sum * math.max(n - 1, 0)
    end
    result.pressure = result.extra * burden / present
    return result
end

local function assisted(mob, assist)
    if assist == nil or next(assist) == nil then
        return mob
    end
    local result = {}
    for key, entry in pairs(mob) do
        result[key] = entry
    end
    result.assist = assist
    return result
end

function M.fight(snap, query, mob, context)
    local result = combat.outcome(snap, query, assisted(mob, context.assist), context.density, context.aid)
    local spent  = result.spent or 0
    if (context.speed or 0) > 0 then
        local own   = result.time < math.huge and 1 / result.time or 0
        local kill  = 1 / (own + context.speed)
        local share = own > 0 and kill / result.time or 0
        local hurt  = (result.taken - spent) * share
        if own == 0 and combat.hits(mob) and (mob.speed or 0) > 0 then
            hurt = combat.blow(snap, mob) / mob.speed * kill
        end
        result = { time = kill, taken = hurt + spent, healed = result.healed * share, drained = result.drained * share }
    end
    local time  = result.time
    local crowd = context.crowd or { pressure = 0, extra = 0 }
    if time == math.huge then
        return {
            time = time,
            taken = math.huge,
            healed = 0,
            drained = result.drained,
            spent = spent,
            ending = -math.huge,
            p_die = 1
        }
    end
    local taken     = result.taken + crowd.pressure * time
    local healed    = math.min(taken, result.healed + (context.support or 0) * time / 1000)
    local low, high = combat.spread(snap, mob)
    local average   = (low + high) / 2
    local hits      = average > 0 and (taken - spent) / average or 0
    return {
        time    = time,
        taken   = taken,
        healed  = healed,
        drained = result.drained,
        spent   = spent,
        ending  = snap.self.hp + healed - taken,
        p_die   = combat.reaches(hits, low, high, snap.self.hp + healed - spent),
    }
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
    local room = data.adjacent - 1
    if extra > room then
        burden = burden * room / extra
        extra  = room
    end
    return { extra = extra, pressure = burden }
end

return M
