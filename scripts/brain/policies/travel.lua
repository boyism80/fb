local combat   = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local danger   = require("brain.policies.danger")
local go       = require("brain.trees.go")
local party    = require("brain.policies.party")
local snapshot = require("brain.snapshot")
local query    = require("brain.query")
local trace    = require("brain.trace")
local value    = require("brain.policies.value")

local M = {}

local function remember_block(brain, bot, event, destination, walls)
    local now   = snapshot.take(brain, bot).now
    local prior = brain:recall(bot, "blocked")
    local since = now
    if type(prior) == "table" and prior.map == event.map and prior.since ~= nil and
        (now - prior.at < combat.rerank or (walls and prior.walls)) then
        since = prior.since
    end
    local dest = {
        map = destination.map,
        maps = destination.maps,
        x = destination.x,
        y = destination.y,
        range = destination.range
    }
    brain:remember(bot, "blocked",
        { detail = event.detail, at = now, since = since, map = event.map, walls = walls, dest = dest })
end

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

-- A blocker I would not survive is waited out or walked around like any other occupant. Clearing took
-- whatever battle.choose returned, which is its best move even when no move survives: a level-5
-- commoner with 50 hp and no weapon swung at a deer (95 hp, 5-15 damage) that stood in its way and
-- had to walk to the shrine (job gather_offering, online 01:57).
local function blocker(brain, bot, event)
    local snap = snapshot.take(brain, bot)
    if event.detail ~= "MOB" or snap.self.ghost then
        return nil
    end
    local facts = query.bind(brain, bot)
    for _, mob in ipairs(snap.world.mobs) do
        if mob.oid == event.oid and mob.hp ~= nil then
            local choice = battle.choose(snap, facts, mob)
            if choice ~= nil and choice.survives then
                return mob,
                    { maneuver = choice.maneuver, key = choice.key, opener = choice.opener, debuff = choice.debuff }
            end
        end
    end
    return nil
end

local function avoided(detour, event)
    for _, tile in ipairs(detour.avoid) do
        if tile.map == event.map and tile.x == event.x and tile.y == event.y then
            return true
        end
    end
    return false
end

local function passable(brain, bot, destination, detour, event)
    local trial = {
        closed = detour.closed,
        perilous = detour.perilous,
        shunned = detour.shunned,
        bounced = detour.bounced,
        avoid = { { map = event.map, x = event.x, y = event.y } }
    }
    if avoided(detour, event) then
        return false
    end
    for _, tile in ipairs(detour.avoid) do
        table.insert(trial.avoid, tile)
    end
    return brain:route(bot, destination, trial) ~= nil
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

-- A tile or gate a mob stood on is given up only while that mob still stands there. Kept for the whole walk,
-- the tiles of rabbits and squirrels that had long wandered off closed all five exits of 고구려왕초보사냥1
-- (danger hit_on_way and recover guarded, online).
local function vacated(brain, bot, detour)
    local standing = {}
    for _, mob in ipairs(brain:mobs(bot)) do
        standing[mob.oid] = mob
    end
    for _, field in ipairs({ "avoid", "closed" }) do
        local kept = {}
        for _, tile in ipairs(detour[field]) do
            local mob = tile.oid ~= nil and standing[tile.oid] or nil
            if tile.oid == nil or (mob ~= nil and mob.x == tile.x and mob.y == tile.y) then
                table.insert(kept, tile)
            end
        end
        detour[field] = kept
    end
end

local function journey(brain, bot, destination, detour)
    while brain:active(bot) do
        vacated(brain, bot, detour)
        go.choose(snapshot.take(brain, bot), query.bind(brain, bot), destination, detour)
        destination.pace = combat.walk
        local event = brain:walk(bot, destination, detour)
        local final = destination.x == event.x and destination.y == event.y
        if event.type == "ARRIVED" then
            return true, event
        elseif event.type == "HOPPED" then
            detour.spent = {}
        elseif event.type == "LEAP_FAILED" then
            detour.spent[event.detail] = true
        elseif event.type == "JUMPED" then
            log("debug", string.format("[brain:travel] bot=%s jumped by %s to map=%d %d,%d", bot:name(), event.detail,
                event.map, event.x, event.y))
        elseif event.type == "OCCUPIED" and passable(brain, bot, destination, detour, event) then
            table.insert(detour.avoid,
                { map = event.map, x = event.x, y = event.y, oid = event.detail == "MOB" and event.oid or nil })
        elseif event.type == "OCCUPIED" and blocker(brain, bot, event) ~= nil then
            local mob, action = blocker(brain, bot, event)
            remember_block(brain, bot, event, destination)
            log("debug", string.format("[brain:travel] bot=%s clears %s %d at map=%d %d,%d", bot:name(), mob.name,
                event.oid, event.map, event.x, event.y))
            local fought = battle.step(brain, bot, event.oid, action)
            if fought.type ~= "KILLED" then
                log("debug", string.format("[brain:travel] bot=%s clearing ended %s", bot:name(), fought.type))
                return false, { type = "ENGAGED", map = event.map, x = event.x, y = event.y, oid = event.oid }
            end
        elseif event.type == "OCCUPIED" then
            local wait = patience(brain, bot, destination, detour, event)
            remember_block(brain, bot, event, destination)
            log("debug", string.format("[brain:travel] bot=%s blocked by %s %d at map=%d %d,%d wait=%dms", bot:name(),
                event.detail, event.oid, event.map, event.x, event.y, wait))
            local target = event.x == event.gx and event.y == event.gy
            local moved  = wait > 0 and brain:watch(bot, event.oid, wait)
            local oid    = event.detail == "MOB" and event.oid or nil
            if moved == false and ((target and final) or (target == false and avoided(detour, event))) then
                return false, event
            elseif moved == false and target then
                detour.crowded = detour.crowded or event.detail == "CHARACTER"
                table.insert(detour.closed, { map = event.map, x = event.x, y = event.y, oid = oid })
            elseif moved == false then
                table.insert(detour.avoid, { map = event.map, x = event.x, y = event.y, oid = oid })
            end
        elseif event.type == "THREATENED" then
            -- A mob I cannot beat stands beside the only way left. The way opens when it moves, so the walk waits
            -- for that instead of closing the exit for good, which kept a bot on 고구려왕초보사냥1 until it ran out
            -- of time (danger hit_on_way, online).
            local wait = patience(brain, bot, destination, detour, event)
            log("debug", string.format("[brain:travel] bot=%s kept off by %d at map=%d %d,%d wait=%dms", bot:name(),
                event.oid, event.map, event.x, event.y, wait))
            if wait <= 0 or brain:watch(bot, event.oid, wait) == false then
                -- A mob that did not move is passed when its blows on the way cost less than staying, and
                -- otherwise the walk is held so the crew comes for me: retried as it was, a poet kept off by a
                -- skeleton that never moved waited 80s while its warrior fought out of its sight (online
                -- group, 16:54).
                local snap = snapshot.take(brain, bot)
                local mob  = nil
                for _, seen in ipairs(snap.world.mobs) do
                    if seen.oid == event.oid then
                        mob = seen
                    end
                end
                if battle.braves(snap, query.bind(brain, bot), mob, event.count or 0) then
                    table.insert(detour.braved, event.oid)
                    log("debug", string.format("[brain:travel] bot=%s braves %d beside %d steps at map=%d %d,%d",
                        bot:name(), event.oid, event.count or 0, event.map, event.x, event.y))
                else
                    remember_block(brain, bot, { map = event.map, detail = "THREAT" }, destination)
                    return false, event
                end
            end
        elseif event.type == "BOUNCED" and detour.bounced[event.dest] == nil then
            log("debug", string.format("[brain:travel] bot=%s bounced off map=%d at map=%d %d,%d", bot:name(),
                event.dest, event.map, event.x, event.y))
            detour.bounced[event.dest] = true
            table.insert(detour.shunned, event.dest)
        elseif event.type == "DENIED" or (event.type == "BLOCKED" and final == false) then
            table.insert(detour.closed, { map = event.map, x = event.x, y = event.y })
        elseif event.type == "NO_ROUTE" and #detour.closed > 0 then
            -- A walk the server keeps turning back is a block like any other: remembered only for occupants and
            -- walls, a bot whose every step was corrected never looked stuck, and its crew crossed without it
            -- (the online form of offline profile pair rooted healer seed 2, 273.9s).
            log("debug", string.format("[brain:travel] bot=%s blocked off at map=%d %d,%d", bot:name(), event.map,
                event.x, event.y))
            remember_block(brain, bot, { map = event.map, detail = detour.crowded and "CHARACTER" or event.detail },
                destination)
            return false, { type = detour.crowded and "CROWDED" or "BLOCKED", map = event.map, x = event.x, y = event.y }
        else
            log("debug", string.format("[brain:travel] bot=%s stopped %s by %s at map=%d %d,%d", bot:name(), event.type,
                tostring(event.detail), event.map, event.x, event.y))
            if event.type == "NO_ROUTE" then
                remember_block(brain, bot, event, destination, true)
            elseif event.type == "BLOCKED" then
                remember_block(brain, bot, event, destination)
            end
            return false, event
        end
    end
    return false, nil
end

function M.go(brain, bot, destination)
    local snap  = snapshot.take(brain, bot)
    local facts = query.bind(brain, bot)
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
        avoid = {},
        threats = danger.threats(snap, facts),
        braved = {},
        perilous = risky and {} or party.perilous(snap, facts),
        shunned = risky and {} or danger.shunned(snap, facts),
        bounced = {}
    }
    expect(brain, bot, destination, detour)
    local arrived, event = journey(brain, bot, destination, detour)
    brain:expect(bot, 0)
    if arrived == false then
        trace.walk(brain, bot, destination, detour, event)
    end
    return arrived, event
end

return M
