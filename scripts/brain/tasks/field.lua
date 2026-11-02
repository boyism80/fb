local combat   = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
local ground   = require("brain.policies.ground")
local recovery = require("brain.policies.recovery")

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
    return #combat.plan(snap, query, mob, danger.guard(mob)).skills > 0
end

local function target(snap, query, names)
    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end
    local found = nil
    local best  = nil
    for _, mob in ipairs(snap.world.mobs) do
        local cost = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
        if wanted[mob.name] and danger.winnable(snap, query, mob) and M.castable(snap, query, mob) and
            (best == nil or cost < best) then
            found = mob
            best  = cost
        end
    end
    return found
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
    return snap.self.ghost == false and snap.self.class ~= "NONE" and snap.record.memory.order == nil and
        (snap.persona or {}).hunt == 1 and ground.choose(snap, query) ~= nil
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
    local mob    = target(snap, query, floor.targets)
    if mob ~= nil then
        return {
            kind = ACTION.ATTACK,
            oid = mob.oid,
            tactic = combat.plan(snap, query, mob, danger.guard(mob)),
            occupy = occupy
        }
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
