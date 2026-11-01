local danger = require("brain.policies.danger")
local means  = require("brain.policies.means")

local M = {}

local function patience(brain, bot, destination, detour, event)
    if event.detail == "MOB" then
        for _, mob in ipairs(brain:mobs(bot)) do
            if mob.oid == event.oid then
                return mob.speed or 0
            end
        end
        return 0
    end

    local step = brain:observe(bot).pace.step or 0
    local base = brain:route(bot, destination, detour)
    local trial = { closed = { { map = event.map, x = event.gx, y = event.gy } } }
    for _, gate in ipairs(detour.closed) do
        table.insert(trial.closed, gate)
    end
    local alternative = brain:route(bot, destination, trial)
    if base == nil then
        return 0
    elseif alternative == nil then
        return base.cost * step
    else
        return math.max(0, alternative.cost - base.cost) * step
    end
end

local function blocker(brain, bot, event)
    if event.detail ~= "MOB" or brain:observe(bot).ghost then
        return nil
    end
    for _, mob in ipairs(brain:mobs(bot)) do
        if mob.oid == event.oid and mob.hp ~= nil and danger.winnable(brain, bot, mob) then
            return mob
        end
    end
    return nil
end

local function expect(brain, bot, destination, detour)
    local step = brain:observe(bot).pace.step
    if step == nil then
        return
    end

    brain:expect(bot, step)
end

function M.go(brain, bot, destination)
    local persona = brain:persona(bot)
    local detour  = { closed = {}, avoid = {}, pace = persona.pace or 0, rush = (persona.rush or 0) > 0 }
    expect(brain, bot, destination, detour)
    while brain:active(bot) do
        detour.jumps = means.jumps(brain, bot)
        local event = brain:walk(bot, destination, detour)
        local final = destination.x == event.x and destination.y == event.y
        if event.type == "ARRIVED" then
            return true, event
        elseif event.type == "JUMPED" then
            log("debug", string.format("[brain:travel] bot=%s jumped by %s to map=%d %d,%d", bot:name(), event.detail,
                event.map, event.x, event.y))
        elseif event.type == "OCCUPIED" and blocker(brain, bot, event) ~= nil then
            local mob = blocker(brain, bot, event)
            brain:remember(bot, "blocked", event.detail)
            log("debug", string.format("[brain:travel] bot=%s clears %s %d at map=%d %d,%d", bot:name(), mob.name,
                event.oid, event.map, event.x, event.y))
            local fought = brain:attack(bot, event.oid,
                { guard = danger.guard(mob), swing = persona.swing or 0, pace = persona.pace or 0 })
            if fought.type == "KILLED" then
                danger.learn(brain, bot, mob, fought.count)
            elseif fought.type == "HURT" then
                return false, fought
            end
        elseif event.type == "OCCUPIED" then
            local wait = patience(brain, bot, destination, detour, event)
            brain:remember(bot, "blocked", event.detail)
            log("debug", string.format("[brain:travel] bot=%s blocked by %s %d at map=%d %d,%d wait=%dms", bot:name(),
                event.detail, event.oid, event.map, event.x, event.y, wait))
            local target = event.x == event.gx and event.y == event.gy
            local moved  = wait > 0 and brain:watch(bot, event.oid, wait)
            if moved == false and target and final then
                return false, event
            elseif moved == false and target then
                table.insert(detour.closed, { map = event.map, x = event.x, y = event.y })
            elseif moved == false then
                table.insert(detour.avoid, { map = event.map, x = event.x, y = event.y })
            end
        elseif event.type == "DENIED" or (event.type == "BLOCKED" and final == false) then
            table.insert(detour.closed, { map = event.map, x = event.x, y = event.y })
        elseif event.type == "NO_ROUTE" and #detour.closed > 0 then
            log("debug", string.format("[brain:travel] bot=%s blocked off at map=%d %d,%d", bot:name(), event.map,
                event.x, event.y))
            return false, { type = "BLOCKED", map = event.map, x = event.x, y = event.y }
        else
            log("debug", string.format("[brain:travel] bot=%s stopped %s at map=%d %d,%d", bot:name(), event.type,
                event.map, event.x, event.y))
            return false, event
        end
    end
    return false, nil
end

return M
