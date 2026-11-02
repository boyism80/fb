local grounds = require("brain.data.grounds")
local hunting = require("brain.data.hunting")
local combat  = require("brain.policies.combat")
local danger  = require("brain.policies.danger")
local score   = require("brain.policies.score")
local value   = require("brain.policies.value")
local field   = require("brain.tasks.field")
local rest    = require("brain.tasks.rest")

local M = {}

local function habitats(query, names)
    local beginner = {}
    for _, name in ipairs(grounds.beginner) do
        beginner[name] = true
    end
    local preferred = {}
    local others    = {}
    for _, habitat in ipairs(query.habitats(names)) do
        table.insert(beginner[habitat.name] and preferred or others, habitat.id)
    end
    return #preferred > 0 and preferred or others
end

local function prey(snap, query, order)
    local wanted = {}
    for _, name in ipairs(order.names) do
        wanted[name] = true
    end
    local found = nil
    local best  = nil
    for _, mob in ipairs(snap.world.mobs) do
        local cost = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
        if wanted[mob.name] and danger.winnable(snap, query, mob) and field.castable(snap, query, mob) and
            (best == nil or cost < best) then
            found = mob
            best  = cost
        end
    end
    return found
end

local function hunt(snap, query, order)
    local spawns = query.spawns(snap.world.map, order.names)
    if #spawns == 0 then
        local maps = habitats(query, order.names)
        if #maps == 0 then
            return { kind = ACTION.FORGET, key = "order" }
        end
        return { kind = ACTION.MOVE, dest = { maps = maps } }
    end
    local mob = prey(snap, query, order)
    if mob ~= nil then
        return {
            kind = ACTION.ATTACK,
            oid = mob.oid,
            tactic = combat.plan(snap, query, mob, danger.guard(mob)),
            tally =
            "order"
        }
    end
    local spot = nil
    local best = nil
    for _, spawn in ipairs(spawns) do
        local cost = math.abs(spawn.x - snap.world.x) + math.abs(spawn.y - snap.world.y)
        if cost > 0 and (best == nil or cost < best) then
            spot = spawn
            best = cost
        end
    end
    if spot == nil then
        local others = {}
        for _, map in ipairs(habitats(query, order.names)) do
            if map ~= snap.world.map then
                table.insert(others, map)
            end
        end
        return #others > 0 and { kind = ACTION.MOVE, dest = { maps = others } } or { kind = ACTION.HOLD }
    end
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = spot.x, y = spot.y, range = 0 } }
end

local function journey(snap, query)
    local action = M.propose(snap, query)
    local worth  = value.baseline(snap, query)
    local trip   = action.kind == ACTION.MOVE and value.trip(snap, query, action.dest, worth) or nil
    return action, worth, trip
end

local function recovering(snap, query, order)
    if order.task ~= "HUNT" or rest.wanted(snap, query) == false then
        return false
    end
    local wanted = {}
    for _, name in ipairs(order.names) do
        wanted[name] = true
    end
    local rested = score.rested(snap)
    for _, mob in pairs(hunting.mobs) do
        if wanted[mob.name] and danger.winnable(snap, query, mob) == false and danger.winnable(rested, query, mob) then
            return true
        end
    end
    return false
end

function M.wanted(snap, query)
    if snap.self.ghost or snap.record.memory.order == nil then
        return false
    elseif recovering(snap, query, snap.record.memory.order) then
        return false
    end
    local _, worth, trip = journey(snap, query)
    return value.patience(trip, worth) <= 0
end

function M.revisit(snap, query)
    if snap.self.ghost or snap.record.memory.order == nil then
        return nil
    end
    local _, worth, trip = journey(snap, query)
    local wait           = value.patience(trip, worth)
    return wait > 0 and wait * 1000 or nil
end

function M.value(snap, query)
    local _, _, trip = journey(snap, query)
    return (snap.record.memory.order.worth or 0) - (trip ~= nil and trip.risk or 0)
end

function M.propose(snap, query)
    local order = snap.record.memory.order
    if order.task == "GO" then
        return { kind = ACTION.MOVE, dest = order.dest, clear = "order" }
    elseif order.task == "HUNT" then
        return hunt(snap, query, order)
    end
    return { kind = ACTION.FORGET, key = "order" }
end

return M
