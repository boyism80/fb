local combat   = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local danger   = require("brain.policies.danger")
local means    = require("brain.policies.means")
local snapshot = require("brain.snapshot")
local query    = require("brain.query")
local value    = require("brain.policies.value")

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

    local step = brain:observe(bot).pace.step or combat.walk
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
    local snap = snapshot.take(brain, bot)
    if event.detail ~= "MOB" or snap.self.ghost then
        return nil
    end
    local facts = query.bind(brain, bot)
    for _, mob in ipairs(snap.world.mobs) do
        if mob.oid == event.oid and mob.hp ~= nil then
            local choice = battle.choose(snap, facts, mob)
            if choice ~= nil then
                return mob, { maneuver = choice.maneuver, key = choice.key, opener = choice.opener }
            end
        end
    end
    return nil
end

function M.yielded(event)
    return event ~= nil and event.type == "ENGAGED"
end

local function expect(brain, bot, destination, detour)
    local step = brain:observe(bot).pace.step
    if step == nil then
        return
    end

    brain:expect(bot, step)
end

local function journey(brain, bot, destination, detour)
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
            local mob, action = blocker(brain, bot, event)
            brain:remember(bot, "blocked", event.detail)
            log("debug", string.format("[brain:travel] bot=%s clears %s %d at map=%d %d,%d", bot:name(), mob.name,
                event.oid, event.map, event.x, event.y))
            local fought = battle.step(brain, bot, event.oid, action)
            if fought.type ~= "KILLED" then
                log("debug", string.format("[brain:travel] bot=%s clearing ended %s", bot:name(), fought.type))
                return false, { type = "ENGAGED", map = event.map, x = event.x, y = event.y, oid = event.oid }
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
                detour.crowded = detour.crowded or event.detail == "CHARACTER"
                table.insert(detour.closed, { map = event.map, x = event.x, y = event.y })
            elseif moved == false then
                table.insert(detour.avoid, { map = event.map, x = event.x, y = event.y })
            end
        elseif event.type == "BLOCKED" and final == false and detour.wary then
            detour.wary  = false
            detour.avoid = {}
        elseif event.type == "BOUNCED" and detour.bounced[event.dest] == nil then
            log("debug", string.format("[brain:travel] bot=%s bounced off map=%d at map=%d %d,%d", bot:name(),
                event.dest, event.map, event.x, event.y))
            detour.bounced[event.dest] = true
            table.insert(detour.shunned, event.dest)
        elseif event.type == "DENIED" or (event.type == "BLOCKED" and final == false) then
            table.insert(detour.closed, { map = event.map, x = event.x, y = event.y })
        elseif event.type == "NO_ROUTE" and #detour.closed > 0 then
            log("debug", string.format("[brain:travel] bot=%s blocked off at map=%d %d,%d", bot:name(), event.map,
                event.x, event.y))
            return false, { type = detour.crowded and "CROWDED" or "BLOCKED", map = event.map, x = event.x, y = event.y }
        else
            log("debug", string.format("[brain:travel] bot=%s stopped %s at map=%d %d,%d", bot:name(), event.type,
                event.map, event.x, event.y))
            return false, event
        end
    end
    return false, nil
end



local function around(snap, facts)
    local tiles = {}
    if snap.self.ghost then
        return tiles
    end
    for _, mob in ipairs(snap.world.mobs) do
        local apart = mob.x ~= nil and
            math.max(math.abs(mob.x - snap.world.x), math.abs(mob.y - snap.world.y)) > combat.reach * 2
        if apart and danger.hostile(snap, facts, mob) then
            for dx = -combat.reach, combat.reach do
                for dy = -combat.reach, combat.reach do
                    if mob.x + dx >= 0 and mob.y + dy >= 0 then
                        table.insert(tiles, { map = snap.world.map, x = mob.x + dx, y = mob.y + dy })
                    end
                end
            end
        end
    end
    return tiles
end

function M.go(brain, bot, destination)
    local snap  = snapshot.take(brain, bot)
    local facts = query.bind(brain, bot)
    local avoid = around(snap, facts)
    local risky = destination.risky
    if risky == nil then
        local choice = value.trip(snap, facts, destination, value.baseline(snap, facts))
        risky = choice ~= nil and choice.risky
        log("debug", string.format("[brain:travel] bot=%s route at map=%d %d,%d: %s", bot:name(), snap.world.map,
            snap.world.x, snap.world.y, choice == nil and "none" or string.format("risky=%s time=%.1f risk=%.1f",
                tostring(choice.risky), choice.time, choice.risk)))
    end
    local detour = {
        closed = {},
        avoid = avoid,
        wary = #avoid > 0,
        perilous = risky and {} or danger.perilous(snap, facts),
        shunned = risky and {} or danger.shunned(snap, facts),
        bounced = {}
    }
    expect(brain, bot, destination, detour)
    local arrived, event = journey(brain, bot, destination, detour)
    brain:expect(bot, 0)
    return arrived, event
end

return M
