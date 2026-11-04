local data     = require("brain.data.combat")
local hunting  = require("brain.data.hunting")
local magic    = require("brain.data.spells")
local combat   = require("brain.policies.combat")
local recovery = require("brain.policies.recovery")

local M = {}

local NEUTRAL = { defense = 0, ward = 0, resists = {}, size = "SMALL" }

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
    local behind = ((magic.back_attack or 1) - 1) / math.max(data.adjacent - 1, 1)
    return combat.blow(snap, mob) / mob.speed * (1 + behind)
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
            local period = M.respawn(mob) / data.rezen
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
    local capacity = math.min(present, data.adjacent)
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

function M.core(snap, query, mob, context)
    local result = combat.outcome(snap, query, assisted(mob, context.assist), context.density, context.aid,
        context.crowd ~= nil and context.crowd.extra or nil)
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
    return { time = result.time, taken = result.taken, healed = result.healed, drained = result.drained, spent = spent }
end

function M.settle(snap, mob, context, result)
    local spent = result.spent or 0
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
    local taken  = result.taken + crowd.pressure * time
    local healed = math.min(taken, result.healed + (context.support or 0) * time / 1000)
    local left   = (snap.self.mp or 0) - (result.drained or 0)
    local mend   = nil
    for _, choice in ipairs(combat.options(snap, NEUTRAL).heal) do
        if choice.hp > 0 and choice.mp > 0 and (mend == nil or choice.hp / choice.mp > mend.hp / mend.mp) then
            mend = choice
        end
    end
    while mend ~= nil and taken - healed >= snap.self.hp and left >= mend.mp do
        healed = healed + mend.hp
        left   = left - mend.mp
    end
    local doses = recovery.doses(snap, "hp")
    local flow  = time > 0 and taken / time or 0
    local cast  = combat.cast(snap)
    for index = #doses, 1, -1 do
        local dose = doses[index]
        local net  = dose.amount - flow * cast
        if taken - healed >= snap.self.hp and net > 0 then
            local count = math.min(dose.count, math.ceil((taken - healed - snap.self.hp + 1) / net))
            healed      = healed + dose.amount * count
            taken       = taken + flow * cast * count
            time        = time + cast * count
        end
    end
    healed          = math.min(taken, healed)
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

local function drinks(snap)
    local stock = 0
    local count = 0
    local room  = recovery.ceiling(snap) - (snap.self.mp or 0)
    for _, dose in ipairs(recovery.doses(snap, "mp")) do
        for _ = 1, dose.count do
            if stock >= room then
                break
            end
            stock = stock + dose.amount
            count = count + 1
        end
    end
    return math.min(stock, room), count
end

local function engage(snap, query, mob, context)
    return M.settle(snap, mob, context, M.core(snap, query, mob, context))
end

function M.fight(snap, query, mob, context)
    local result = engage(snap, query, mob, context)
    if result.ending > 0 or recovery.rates(snap).mp > 0 then
        return result
    end
    local stock, count = drinks(snap)
    if stock <= 0 then
        return result
    end
    local delay   = count * combat.cast(snap)
    local hurt    = (combat.hits(mob) and (mob.speed or 0) > 0 and combat.blow(snap, mob) / mob.speed or 0) * delay
    local drunk   = setmetatable({
        self = setmetatable({ hp = snap.self.hp - hurt, mp = (snap.self.mp or 0) + stock },
            { __index = snap.self })
    }, { __index = snap })
    local refined = hurt < snap.self.hp and engage(drunk, query, mob, context) or nil
    if refined == nil or refined.ending <= result.ending then
        return result
    end
    refined.time  = refined.time + delay
    refined.taken = refined.taken + hurt
    return refined
end

local approaching = setmetatable({}, { __mode = "k" })

local function approach(snap)
    local world = snap.world
    local byspell = approaching[world]
    if byspell == nil then
        byspell = setmetatable({}, { __mode = "k" })
        approaching[world] = byspell
    end
    local buffs = combat.buffs(snap)
    local found = byspell[buffs]
    if found == nil or found.phydef ~= snap.self.phydef then
        found = { phydef = snap.self.phydef }
        for _, seen in ipairs(world.mobs or {}) do
            if seen.x ~= nil and seen.attack == "AGGRESSIVE" and combat.hits(seen) and (seen.speed or 0) > 0 then
                local distance = math.abs(seen.x - world.x) + math.abs(seen.y - world.y)
                table.insert(found, {
                    oid = seen.oid,
                    reached = math.max(0, distance - data.reach) * seen.speed,
                    pressure = pressure(snap, seen)
                })
            end
        end
        byspell[buffs] = found
    end
    return found
end

function M.contact(snap, mob, time)
    local burden = 0
    local extra  = 0
    for _, seen in ipairs(approach(snap)) do
        if seen.oid ~= mob.oid then
            local share = time == math.huge and 1 or time > 0 and math.max(0, time - seen.reached) / time or 0
            extra       = extra + share
            burden      = burden + share * seen.pressure
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
