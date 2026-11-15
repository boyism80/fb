local data  = require("brain.data.combat")
local value = require("brain.policies.value")

local M = {}

local LIMIT = 64

-- A walk stopped at a bot of ours in a narrow way is untangled for every bot of ours round it at once: their tiles,
-- the tiles their walks head for and who goes first make one plan of steps, solved in one pass (atlas untangle,
-- PIBT), which each of them then walks in step (follow). Left to each to step aside on its own, the one asked to make
-- way found its own way out taken by a third, and two crews stood in the way down to 흑해골굴 from 국내성 for ten
-- minutes (online 170334, 17:05-17:15; user 11-15: one computation moves every bot that has to). The bot whose walk
-- stopped goes first, a bot walking somewhere next, and a bot waiting moves only when pushed.
-- A bot already in a plan on this map is moved by that plan, never by a second one made over it: made by both bots of a
-- swap at once, each plan wiped the other's, the steps walked never matched, and a poet planned 39 times in ten minutes
-- on 평양흑해골굴 (online 200750, 20:17:07). In one myself, I end my walk to follow it; with another bot in one, I wait
-- for that plan to end as I wait for any bot in my way.
local function engaged(query, map, name)
    local plan = query.directed(name)
    return type(plan) == "table" and plan.map == map
end

function M.untangle(brain, bot, snap, query, destination, event)
    if event.gx == nil or snap.world.x == nil or query.untangle == nil then
        return false
    end
    if engaged(query, snap.world.map, snap.self.name) then
        return true
    end
    local radius = math.max(data.sight.x, data.sight.y)
    local movers = { {
        x = snap.world.x,
        y = snap.world.y,
        gx = event.gx,
        gy = event.gy,
        range = destination.range or 0,
        priority = 2
    } }
    local names  = { snap.self.name }
    local listed = { [snap.self.name] = true }
    local taken  = { [snap.world.x .. "," .. snap.world.y] = true }
    local held   = false
    local function join(name, at, going)
        if listed[name] ~= nil or at == nil or at.x == nil or math.abs(at.x - event.x) + math.abs(at.y - event.y) > radius then
            return
        end
        local walking = type(going) == "table" and going.x ~= nil
        table.insert(movers, {
            x = at.x,
            y = at.y,
            gx = walking and going.x or nil,
            gy = walking and going.y or nil,
            range = 0,
            priority = walking and 1 or 0,
        })
        table.insert(names, name)
        listed[name]               = true
        taken[at.x .. "," .. at.y] = true
        held                       = held or (at.x == event.x and at.y == event.y)
    end
    for _, entry in ipairs(query.roster()) do
        local profile = entry.profile
        local ally    = query.ally(entry.name)
        local world   = type(profile) == "table" and profile.world or nil
        local going   = type(profile) == "table" and profile.going or nil
        if type(profile) == "table" and profile.self ~= nil and profile.self.ghost == false and
            (ally ~= nil or (world ~= nil and world.map == snap.world.map)) then
            join(entry.name, ally ~= nil and ally.x ~= nil and ally or world,
                type(going) == "table" and going.map == snap.world.map and going or nil)
        end
    end
    for _, entry in ipairs(query.presences ~= nil and query.presences(snap.self.name) or {}) do
        local ally = query.ally(entry.name)
        if entry.map == snap.world.map then
            join(entry.name, ally ~= nil and ally.x ~= nil and ally or entry,
                entry.gx ~= nil and { x = entry.gx, y = entry.gy } or nil)
        end
    end
    if held == false then
        return false
    end
    for _, name in ipairs(names) do
        if engaged(query, snap.world.map, name) then
            return false
        end
    end
    local blocked = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and taken[mob.x .. "," .. mob.y] == nil then
            table.insert(blocked, { x = mob.x, y = mob.y })
        end
    end
    for _, tile in ipairs(snap.world.occupied or {}) do
        if taken[tile.x .. "," .. tile.y] == nil then
            table.insert(blocked, { x = tile.x, y = tile.y })
        end
    end
    local paths = query.untangle(snap.world.map, movers, blocked, LIMIT)
    if paths == nil then
        return false
    end
    local id    = snap.self.name .. "@" .. tostring(snap.now)
    local parts = {}
    for index, path in ipairs(paths) do
        query.direct(names[index],
            { id = id, map = snap.world.map, steps = path, done = 1, made = snap.now, members = names })
        local last = path[#path] or {}
        table.insert(parts, string.format("%s %d,%d>%s,%s/%d", names[index], movers[index].x, movers[index].y,
            tostring(last.x), tostring(last.y), #path))
    end
    log("debug", string.format("[brain:traffic] bot=%s plans %s at map=%d: %s", snap.self.name, id, snap.world.map,
        table.concat(parts, "; ")))
    return true
end

-- Where a bot stands and where its walk heads are told to every agent while it stands where a plan may be needed: its
-- walk stopped, a plan moves it, or it waits by a warp. Told for every step of every bot, two thousand bots would
-- flood the master; told only here, a bot of another agent is moved by the plan as a bot of mine is.
function M.present(snap, facts, decision, posted)
    if facts.present == nil then
        return
    end
    local here    = snap.world
    local planned = facts.directed ~= nil and facts.directed(snap.self.name) ~= nil
    local blocked = type(posted) == "table" and posted.blocker ~= nil
    local near    = false
    if decision ~= nil and decision.kind == ACTION.HOLD and here.x ~= nil and facts.warp ~= nil then
        for dx = -data.adjacent, data.adjacent do
            for dy = -(data.adjacent - math.abs(dx)), data.adjacent - math.abs(dx) do
                near = near or facts.warp(here.map, here.x + dx, here.y + dy) == true
            end
        end
    end
    local going = type(posted) == "table" and posted.going or nil
    if here.x ~= nil and snap.self.ghost == false and (planned or near or blocked) then
        facts.present(snap.self.name, {
            map = here.map,
            x = here.x,
            y = here.y,
            gx = type(going) == "table" and going.map == here.map and going.x or nil,
            gy = type(going) == "table" and going.map == here.map and going.y or nil,
        })
    else
        facts.present(snap.self.name, nil)
    end
end

local waits = {}

local function same(step, x, y)
    return step ~= nil and step.x == x and step.y == y
end

-- A plan is mine while I stand on its map and no step failed since it was made. A failed step drops it, so no bot of
-- the plan waits on a part that is not walked any more, and the walk that failed asks for a new plan from where all
-- stand now. The failure is ordered against the plan by time, not by the retry wait still running: the wait is gone
-- in 100ms to a few seconds, a mage that next decided seconds later found none and stepped seven times into a poet
-- that had left the plan, while its warrior waited 29s before the door (online 20261115-125736, 13:00:22-13:00:51).
function M.planned(snap, facts)
    local plan = facts.directed ~= nil and facts.directed(snap.self.name) or nil
    if type(plan) ~= "table" then
        return false
    end
    local record = snap.record.failures["TRAFFIC"]
    local failed = record ~= nil and snap.now - record.age >= (plan.made or 0)
    if plan.map ~= snap.world.map or failed then
        log("debug", string.format("[brain:traffic] bot=%s drops %s at %s,%s done=%s/%d", snap.self.name,
            tostring(plan.id), tostring(snap.world.x), tostring(snap.world.y), tostring(plan.done), #(plan.steps or {})))
        facts.direct(snap.self.name, nil)
        return false
    end
    return true
end

-- It is my turn to step: my part has a step left and no bot of the plan is still to walk before it.
function M.stepping(snap, facts)
    local action = M.follow(snap, facts)
    return action ~= nil and action.kind == ACTION.MOVE
end

-- My next step keeps every bot of the plan still walking on its way, so whatever else I do first costs each of them
-- the time of that action. Ordered before every other answer, a poet waited on its plan 14s beside its vanguard
-- falling to two mobs without one heal, and the vanguard died (online 180725, 18:14:14-18:14:30; user 11-14:
-- survival comes first).
function M.worth(snap, facts)
    local plan    = facts.directed(snap.self.name)
    local walking = 0
    for _, name in ipairs(type(plan) == "table" and plan.members or {}) do
        local other = facts.directed(name)
        if type(other) == "table" and other.id == plan.id and (other.done or 1) < #(other.steps or {}) then
            walking = walking + 1
        end
    end
    return walking * (snap.pace.swing or data.swing) / 1000 * value.baseline(snap, facts).value
end

-- My part of a plan is walked one step at a time, a step once every other bot of the plan has walked the one before:
-- walked ahead, a bot that only had to wait reached a tile another had yet to pass through. Steps that keep me where I
-- am are passed at once, and only a bot in sight and on its own part is waited for, as its next step is what wakes me:
-- waiting on one out of sight, a poet held in 국내성 for minutes after its rogue had crossed (online 174502, 17:46:27).
-- A bot whose part stepped it aside keeps that tile while any other bot of the plan walks: back on the tile it waited
-- on as soon as its own part ended, a poet took again the tile its rogue had to step to, and their vanguard stood 45s
-- before the door (online 180725, 18:09:41-18:10:21). A part that ends on a warp ends one step before it, and my own
-- walk takes me through: walked as a step on this map, the walk went on after the warp and came back to its tile, by a
-- 비영사천문 that left the vanguard 123 steps away (online 180725, 18:10:21).
function M.follow(snap, facts)
    if facts.directed == nil or snap.world.x == nil then
        return nil
    end
    local plan = facts.directed(snap.self.name)
    if type(plan) ~= "table" or type(plan.steps) ~= "table" then
        return nil
    end
    local steps = plan.steps
    local done  = plan.done or 1
    local last  = #steps
    if plan.map ~= snap.world.map then
        facts.direct(snap.self.name, nil)
        return nil
    end
    if last > 1 and facts.warp ~= nil and facts.warp(plan.map, steps[last].x, steps[last].y) == true then
        last = last - 1
    end
    local moved = false
    while done < last and same(steps[done + 1], snap.world.x, snap.world.y) do
        done  = done + 1
        moved = true
    end
    if same(steps[done], snap.world.x, snap.world.y) == false then
        facts.direct(snap.self.name, nil)
        return nil
    end
    if moved then
        plan.done = done
        facts.direct(snap.self.name, plan)
    end
    local finished = done >= last
    local aside    = finished and last == #steps
    for _, name in ipairs(plan.members or {}) do
        local other = name ~= snap.self.name and facts.directed(name) or nil
        local seen  = type(other) == "table" and facts.ally(name) or nil
        if seen ~= nil and seen.x ~= nil and other.id == plan.id and (aside or (other.done or 1) < done) and
            (other.done or 1) < #(other.steps or {}) and same((other.steps or {})[other.done or 1], seen.x, seen.y) then
            local reason = string.format("%s for %s at %d,%d done=%d/%d mine=%d/%d aside=%s", tostring(plan.id), name,
                seen.x, seen.y, other.done or 1, #(other.steps or {}), done, #steps, tostring(aside))
            if waits[snap.self.name] ~= reason then
                waits[snap.self.name] = reason
                log("debug", string.format("[brain:traffic] bot=%s waits %s", snap.self.name, reason))
            end
            return { kind = ACTION.HOLD }
        end
    end
    if finished then
        facts.direct(snap.self.name, nil)
        return nil
    end
    -- A step onto a tile a bot of the plan leaves at that same step waits until it has left: walked at once, a warrior
    -- reached 2,12 before its poet had stepped off it to 3,12, the step failed and the plan was dropped, and the crew
    -- stood in the one-tile corridor of 흑령굴 for five minutes (online 004028, 00:43:20).
    local target = steps[done + 1]
    for _, name in ipairs(plan.members or {}) do
        local other = name ~= snap.self.name and facts.directed(name) or nil
        local seen  = type(other) == "table" and other.id == plan.id and (other.done or 1) == done and facts.ally(name) or
            nil
        if seen ~= nil and seen.x == target.x and seen.y == target.y then
            local reason = string.format("%s for %s to leave %d,%d mine=%d/%d", tostring(plan.id), name, target.x,
                target.y, done, #steps)
            if waits[snap.self.name] ~= reason then
                waits[snap.self.name] = reason
                log("debug", string.format("[brain:traffic] bot=%s waits %s", snap.self.name, reason))
            end
            return { kind = ACTION.HOLD }
        end
    end
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = target.x, y = target.y, range = 0, traffic = plan.id } }
end

return M
