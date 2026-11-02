local combat   = require("brain.policies.combat")
local data     = require("brain.data.combat")
local growth   = require("brain.data.growth")
local hunting  = require("brain.data.hunting")
local teleport = require("brain.data.teleport")
local score    = require("brain.policies.score")

local M = {}

local perils   = {}
local revivals = {}
local typical  = nil

local function respawn(seen)
    local mob = hunting.mobs[seen.mob]
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

function M.period(seen)
    return respawn(seen)
end

function M.weight(snap, seen)
    local period = respawn(seen)
    if period <= 0 then
        return 0
    end
    return 0.5 ^ (math.max(0, snap.now - seen.at) / period)
end

function M.chance(snap, query, mob, expose)
    if expose <= 0 or M.winnable(snap, query, mob) then
        return 0
    end
    local blow = combat.blow(snap, mob)
    if blow <= 0 then
        return 0
    end
    local die = snap.self.hp / blow * (mob.speed or data.swing)
    return math.min(1, expose / math.max(die, 1))
end

function M.revival(snap, query)
    local key = table.concat({ snap.world.map, snap.self.nation or 0 }, "|")
    if revivals[key] == nil then
        local maps    = teleport.returns[snap.self.nation] or {}
        local route   = #maps > 0 and query.route(maps) or nil
        revivals[key] = route ~= nil and route.cost or false
    end
    if revivals[key] == false then
        return 0
    end
    return revivals[key] * (snap.pace.step or data.walk) * 2 / 1000
end

function M.loss(snap, query, worth)
    local levels = growth[snap.self.class]
    local now    = levels ~= nil and levels[snap.self.level] or nil
    local after  = levels ~= nil and levels[snap.self.level + 1] or nil
    local exp    = 0
    if now ~= nil and after ~= nil then
        exp = math.min(math.max(0, (snap.self.exp or 0) - now.exp), (after.exp - now.exp) * data.penalty.exp)
    end
    local wear = 0
    for _, slot in ipairs(snap.gear or {}) do
        for _, name in ipairs(slot.names) do
            local item = query.item(name)
            wear = wear + (item ~= nil and item.price or 0) * data.penalty.durability
        end
    end
    return exp + M.revival(snap, query) * worth.value + wear * worth.exchange
end

function M.combine(chances)
    local survive = 1
    for _, chance in ipairs(chances) do
        survive = survive * (1 - chance)
    end
    return 1 - survive
end

function M.winnable(snap, query, mob)
    return combat.fight(snap, query, mob).margin > 0
end

function M.hostile(snap, query, mob)
    return mob.attack == "AGGRESSIVE" and M.winnable(snap, query, mob) == false
end

function M.deadly(snap, query, mob)
    if mob.attack ~= "AGGRESSIVE" or M.winnable(snap, query, mob) then
        return false
    end
    local step   = snap.pace.step or data.walk
    local chases = mob.speed ~= nil and mob.speed <= step
    local lethal = mob.damage ~= nil and combat.mitigate(mob.damage, snap.self.phydef or 0, 0) >= snap.self.hp
    return chases or lethal
end

function M.perilous(snap, query)
    if snap.self.ghost then
        return {}
    end
    local key = score.signature(snap)
    if perils[key] == nil then
        local found  = {}
        local rested = score.rested(snap)
        for id, mob in pairs(hunting.mobs) do
            if M.deadly(rested, query, mob) then
                table.insert(found, id)
            end
        end
        perils[key] = found
    end
    return perils[key]
end

function M.shunned(snap, query)
    if snap.self.ghost then
        return {}
    end
    local known  = hunting.mobs
    local result = {}
    for map, seen in pairs(snap.record.memory.perils or {}) do
        local mob    = type(seen) == "table" and (known[seen.mob] or seen.stats) or nil
        local id     = tonumber(map)
        local recent = mob ~= nil and snap.now - seen.at < respawn(seen)
        if mob ~= nil and ((recent and M.hostile(snap, query, mob)) or
                (M.deadly(snap, query, mob) and query.native(id, mob.id))) then
            table.insert(result, id)
        end
    end
    return result
end

function M.guard(mob)
    return combat.hits(mob) and mob.damage or 0
end

function M.threat(snap)
    local threat = 0
    for _, mob in ipairs(snap.world.mobs) do
        if mob.attack == "AGGRESSIVE" then
            threat = math.max(threat, combat.blow(snap, mob))
        end
    end
    return threat
end

function M.fatal(snap, query)
    for _, mob in ipairs(snap.world.mobs) do
        local near = mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
        if M.deadly(snap, query, mob) or (near and mob.attack == "AGGRESSIVE" and M.winnable(snap, query, mob) == false) then
            return mob
        end
    end
    return nil
end

return M
