local combat   = require("brain.policies.combat")
local data     = require("brain.data.combat")
local growth   = require("brain.data.growth")
local hunting  = require("brain.data.hunting")
local teleport = require("brain.data.teleport")
local memo     = require("brain.policies.memo")
local score    = require("brain.policies.score")
local survival = require("brain.policies.survival")

local M = {}

local perils   = {}
local revivals = {}

local function respawn(seen)
    return survival.respawn(hunting.mobs[seen.mob])
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
    if expose <= 0 then
        return 0
    end
    local window = M.winnable(snap, query, mob) and math.min(expose, combat.outcome(snap, query, mob).time) or expose
    return combat.lethal(snap, mob, window, snap.self.hp)
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
    local result = combat.outcome(snap, query, mob)
    local crowd  = mob.oid ~= nil and survival.contact(snap, mob, result.time) or { pressure = 0 }
    return survival.ending(snap, result, crowd) > 0
end

function M.penalty(snap)
    local levels = growth[snap.self.class]
    local now    = levels ~= nil and levels[snap.self.level] or nil
    local after  = levels ~= nil and levels[snap.self.level + 1] or nil
    if now == nil or after == nil then
        return 0
    end
    return (after.exp - now.exp) * data.penalty.exp
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
    return memo.get(perils, snap.self.name, score.signature(snap, ""), score.signature(snap), function()
        local found  = {}
        local rested = score.rested(snap)
        for id, mob in pairs(hunting.mobs) do
            if M.deadly(rested, query, mob) then
                table.insert(found, id)
            end
        end
        return found
    end)
end

local shunning = setmetatable({}, { __mode = "k" })

function M.shunned(snap, query)
    if snap.self.ghost then
        return {}
    end
    if shunning[snap] == nil then
        local known = hunting.mobs
        local found = {}
        for map, seen in pairs(snap.record.memory.perils or {}) do
            local mob    = type(seen) == "table" and (known[seen.mob] or seen.stats) or nil
            local id     = tonumber(map)
            local recent = mob ~= nil and snap.now - seen.at < respawn(seen)
            if mob ~= nil and ((recent and M.hostile(snap, query, mob)) or
                    (M.deadly(snap, query, mob) and query.native(id, mob.id))) then
                table.insert(found, id)
            end
        end
        shunning[snap] = found
    end
    return table.move(shunning[snap], 1, #shunning[snap], 1, {})
end

function M.threat(snap, maps)
    local threat = 0
    for _, mob in ipairs(snap.world.mobs) do
        if mob.attack == "AGGRESSIVE" then
            threat = math.max(threat, combat.blow(snap, mob))
        end
    end
    local perils = snap.record.memory.perils or {}
    for _, map in ipairs(maps or { snap.world.map }) do
        local seen = perils[tostring(map)] or perils[map]
        if type(seen) == "table" and type(seen.stats) == "table" and seen.stats.attack == "AGGRESSIVE" then
            threat = math.max(threat, M.weight(snap, seen) * combat.blow(snap, seen.stats))
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
