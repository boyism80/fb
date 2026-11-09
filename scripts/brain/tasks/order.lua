local data    = require("brain.data.combat")
local grounds = require("brain.data.grounds")
local hunting = require("brain.data.hunting")
local battle  = require("brain.policies.battle")
local combat  = require("brain.policies.combat")
local danger  = require("brain.policies.danger")
local score   = require("brain.policies.score")
local value   = require("brain.policies.value")
local field   = require("brain.tasks.field")

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

local function hunt(snap, query, order)
    local spawns = query.spawns(snap.world.map, order.names)
    if #spawns == 0 then
        local maps = habitats(query, order.names)
        if #maps == 0 then
            return { kind = ACTION.FORGET, key = "order" }
        end
        return { kind = ACTION.MOVE, dest = { maps = maps } }
    end
    local found = field.target(snap, query, order.names)
    if found ~= nil then
        return battle.propose(found, { tally = "order" })
    end
    local spot  = nil
    local best  = nil
    local range = math.min(data.sight.x, data.sight.y)
    for _, spawn in ipairs(spawns) do
        local cost = math.abs(spawn.x - snap.world.x) + math.abs(spawn.y - snap.world.y)
        if cost > range and (best == nil or cost < best) then
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
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = spot.x, y = spot.y, range = range } }
end

local function journey(snap, query)
    local order  = snap.record.memory.order
    local action = order.task == "GO" and M.go(snap) or order.task == "HUNT" and M.hunt(snap, query) or M.forget()
    local worth  = value.baseline(snap, query)
    local trip   = action.kind == ACTION.MOVE and value.trip(snap, query, action.dest, worth) or nil
    return action, worth, trip
end

local names = nil

local function named()
    if names == nil then
        names = {}
        for _, mob in pairs(hunting.mobs) do
            names[mob.name] = names[mob.name] or {}
            table.insert(names[mob.name], mob)
        end
    end
    return names
end

local function recovering(snap, query, order)
    if order.task ~= "HUNT" then
        return false
    end
    local rested = score.rested(snap)
    for _, name in ipairs(order.names) do
        local least = math.huge
        local weak  = nil
        for _, mob in ipairs(named()[name] or {}) do
            local spike = combat.hits(mob) and combat.spike(snap, { { mob = mob, count = 1 } }) or 0
            if spike < least then
                least = spike
                weak  = mob
            end
        end
        if weak ~= nil and snap.self.hp < snap.self.max_hp and (snap.self.hp <= least or
                (danger.winnable(snap, query, weak) == false and danger.winnable(rested, query, weak))) then
            return true
        end
    end
    return false
end

function M.recovering(snap, query)
    return recovering(snap, query, snap.record.memory.order)
end

function M.wait_time(snap, query)
    local _, worth, trip = journey(snap, query)
    return value.patience(trip, worth)
end

function M.value(snap, query)
    local _, _, trip = journey(snap, query)
    return (snap.record.memory.order.worth or 0) - (trip ~= nil and trip.risk or 0)
end

function M.go(snap)
    return { kind = ACTION.MOVE, dest = snap.record.memory.order.dest, clear = "order" }
end

function M.hunt(snap, query)
    return hunt(snap, query, snap.record.memory.order)
end

function M.forget()
    return { kind = ACTION.FORGET, key = "order" }
end

return M
