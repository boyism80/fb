local resp     = require("integration.response")
local fight    = require("brain.policies.combat")
local danger   = require("brain.policies.danger")
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

function M.go(brain, bot, goal)
    local skipped = {}
    local visited = {}
    local tried   = {}
    while brain:active(bot) and goal.done() == false do
        if goal.full ~= nil and goal.full() then
            return false
        end

        local here   = bot:position()
        local x, y   = here[1], here[2]
        local target = nil
        local best   = nil
        local risky  = false
        local snap   = snapshot.take(brain, bot)
        local facts  = query.bind(brain, bot)
        for _, mob in ipairs(snap.world.mobs) do
            local cost   = math.abs(mob.x - x) + math.abs(mob.y - y)
            local wanted = goal.wants(mob.name) and skipped[mob.oid] == nil
            local safe   = wanted and danger.winnable(snap, facts, mob)
            risky        = risky or (wanted and safe == false)
            if safe and (best == nil or cost < best) then
                target = mob
                best   = cost
            end
        end

        local drop = M.drop(brain, bot, goal.loot, tried)
        if drop ~= nil then
            tried[drop.oid] = true
            if travel.go(brain, bot, { map = bot:map(), x = drop.x, y = drop.y, range = 0 }) and M.present(brain, bot, drop) then
                brain:loot(bot)
            end
        elseif target == nil and risky then
            log("debug", string.format("[brain:hunt] bot=%s too weak for visible prey", bot:name()))
            return false
        elseif target ~= nil then
            local tactic = fight.plan(snap, facts, target, danger.guard(target))
            local event  = brain:attack(bot, target.oid, tactic)
            if event.type == "KILLED" then
                goal.killed(target)
            elseif event.type == "LOST" or event.type == "UNREACHABLE" then
                skipped[target.oid] = true
            else
                log("debug", string.format("[brain:hunt] bot=%s stopped %s", bot:name(), event.type))
                return false
            end
        else
            local area = nil
            local near = nil
            for index, spawn in ipairs(brain:spawns(bot, goal.names)) do
                local cost = math.abs(spawn.x - x) + math.abs(spawn.y - y)
                if visited[index] == nil and (near == nil or cost < near) then
                    area = { index = index, x = spawn.x, y = spawn.y }
                    near = cost
                end
            end

            if area == nil and next(visited) == nil then
                log("debug", string.format("[brain:hunt] bot=%s has no spawn area on map=%d", bot:name(), bot:map()))
                return false
            elseif area == nil then
                visited = {}
                wait.reply(bot, resp.update, nil, function() return true end, APPEAR_WAIT)
            else
                visited[area.index] = true
                travel.go(brain, bot, { map = bot:map(), x = area.x, y = area.y, range = 0 })
            end
        end
    end
    return goal.done()
end

return M
