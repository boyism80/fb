local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local ground   = require("brain.policies.ground")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")

local M = {}

local function position(snap, floors)
    for index, floor in ipairs(floors) do
        if floor.map == snap.world.map then
            return index
        end
    end
    return nil
end

function M.castable(snap, query, mob)
    local least = recovery.least(snap)
    if least == nil or (snap.self.mp or 0) < least or snap.self.class == "WARRIOR" then
        return true
    end
    for _, choice in ipairs(combat.options(snap, mob).offense) do
        if choice.damage > 0 then
            return true
        end
    end
    return false
end

function M.target(snap, query, names)
    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end
    return battle.target(snap, query, function(mob)
        return wanted[mob.name] ~= nil and M.castable(snap, query, mob)
    end)
end

local function area(snap, query, names)
    local found = nil
    local best  = nil
    for _, spawn in ipairs(query.spawns(snap.world.map, names)) do
        local cost = math.abs(spawn.x - snap.world.x) + math.abs(spawn.y - snap.world.y)
        if cost > 0 and (best == nil or cost < best) then
            found = spawn
            best  = cost
        end
    end
    return found
end

function M.wanted(snap, query)
    if snap.self.ghost then
        return false
    end
    local objective = value.objective(snap, query)
    return objective.task == "FIELD" and objective.dest ~= nil
end

function M.propose(snap, query)
    local verdict = ground.ready(snap, query)
    local best    = verdict.best
    local index   = position(snap, best.floors)
    if index == nil then
        return { kind = ACTION.MOVE, dest = { maps = { best.floors[1].map } } }
    end

    local floor  = best.floors[index]
    local occupy = { map = floor.map, rate = floor.kills }
    local found  = M.target(snap, query, floor.targets)
    if found ~= nil then
        return battle.propose(found, { occupy = occupy })
    elseif #best.floors > 1 then
        local next = best.floors[index % #best.floors + 1]
        return { kind = ACTION.MOVE, dest = { maps = { next.map } }, occupy = occupy }
    end
    local spot = area(snap, query, floor.targets)
    if spot ~= nil then
        return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = spot.x, y = spot.y, range = 0 }, occupy = occupy }
    end
    return { kind = ACTION.HOLD }
end

return M
