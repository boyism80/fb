local resp     = require("integration.response")
local battle   = require("brain.policies.battle")
local travel   = require("brain.policies.travel")
local wait     = require("brain.policies.wait")
local snapshot = require("brain.snapshot")
local query    = require("brain.query")
local grounds  = require("brain.data.grounds")

local M = {}

function M.beginner(brain, bot, names)
    local beginner = {}
    for _, name in ipairs(grounds.beginner) do
        beginner[name] = true
    end
    local maps = {}
    for _, habitat in ipairs(brain:habitats(bot, names)) do
        if beginner[habitat.name] then
            table.insert(maps, habitat.id)
        end
    end
    return maps
end

local APPEAR_WAIT = 10000

function M.drop(brain, bot, names, tried)
    if names == nil then
        return nil
    end

    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end

    local here    = bot:position()
    local nearest = nil
    local best    = nil
    for _, item in ipairs(brain:items(bot)) do
        local match = false
        for _, name in ipairs(item.names) do
            match = match or wanted[name] == true
        end
        local cost = math.abs(item.x - here[1]) + math.abs(item.y - here[2])
        if match and tried[item.oid] == nil and (best == nil or cost < best) then
            nearest = item
            best    = cost
        end
    end
    return nearest
end

function M.present(brain, bot, drop)
    for _, item in ipairs(brain:items(bot)) do
        if item.oid == drop.oid then
            return true
        end
    end
    return false
end

local function load(brain, bot, goal)
    local state = brain:recall(bot, "hunt")
    if state == nil or state.key ~= goal.key then
        state = { key = goal.key, counts = {}, skipped = {}, visited = {}, tried = {} }
    end
    return state
end

function M.step(brain, bot, goal)
    local state = load(brain, bot, goal)
    local save  = function()
        brain:remember(bot, "hunt", state)
    end
    if goal.done(state) then
        brain:remember(bot, "hunt", nil)
        return "DONE"
    elseif goal.full ~= nil and goal.full() then
        return "FAILED", "FULL"
    end

    local here   = bot:position()
    local snap   = snapshot.take(brain, bot)
    local facts  = query.bind(brain, bot)
    local risky  = false
    local accept = function(mob)
        return goal.wants(state, mob.name) and state.skipped[tostring(mob.oid)] == nil
    end
    for _, mob in ipairs(snap.world.mobs) do
        risky = risky or accept(mob)
    end
    local found = battle.target(snap, facts, accept)
    local drop  = M.drop(brain, bot, goal.loot, state.tried)
    if drop ~= nil then
        state.tried[tostring(drop.oid)] = true
        save()
        if travel.go(brain, bot, { map = bot:map(), x = drop.x, y = drop.y, range = 0 }) and M.present(brain, bot, drop) then
            brain:loot(bot)
        end
        return "PROGRESS"
    elseif found == nil and risky then
        log("debug", string.format("[brain:hunt] bot=%s too weak for visible prey", bot:name()))
        return "FAILED", "UNSAFE"
    elseif found ~= nil then
        local event = battle.step(brain, bot, found.mob.oid, battle.propose(found))
        if event.type == "KILLED" then
            goal.killed(state, found.mob)
        elseif event.type == "LOST" or event.type == "UNREACHABLE" then
            state.skipped[tostring(found.mob.oid)] = true
        elseif battle.going(event.type) == false then
            log("debug", string.format("[brain:hunt] bot=%s stopped %s", bot:name(), event.type))
            return "FAILED", event.type
        end
        save()
        return goal.done(state) and "DONE" or "PROGRESS"
    end

    local area = nil
    local near = nil
    for index, spawn in ipairs(brain:spawns(bot, goal.names)) do
        local cost = math.abs(spawn.x - here[1]) + math.abs(spawn.y - here[2])
        if state.visited[tostring(index)] == nil and (near == nil or cost < near) then
            area = { index = index, x = spawn.x, y = spawn.y }
            near = cost
        end
    end
    if area == nil and next(state.visited) == nil then
        log("debug", string.format("[brain:hunt] bot=%s has no spawn area on map=%d", bot:name(), bot:map()))
        return "FAILED", "NO_SPAWN"
    elseif area == nil then
        state.visited = {}
        save()
        wait.reply(bot, resp.update, nil, function() return true end, APPEAR_WAIT)
        return "PROGRESS"
    end
    state.visited[tostring(area.index)] = true
    save()
    travel.go(brain, bot, { map = bot:map(), x = area.x, y = area.y, range = 0 })
    return "PROGRESS"
end

return M
