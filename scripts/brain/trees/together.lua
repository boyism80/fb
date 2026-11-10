local data     = require("brain.data.combat")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local spacing  = require("brain.policies.spacing")
local tree     = require("brain.tree")

local choose, when, act = tree.select, tree.when, tree.act

local M = {}

local function floor_of(snap, plan, map)
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        if floor.map == map then
            return floor
        end
    end
    return nil
end

function M.on_ground(snap, facts)
    return floor_of(snap, party.plan(snap, facts), snap.world.map) ~= nil
end

function M.grouped(snap)
    return #(snap.party or {}) > 0
end

function M.apart(snap, facts)
    return M.grouped(snap) and escort.complete(snap, facts) == false
end

-- Moving while the group is scattered always goes through the rally point the leader chose
-- (party.rally). This is the only place that decides where scattered members meet; see party.rally
-- for why the per-branch choices it replaced bounced and deadlocked.
function M.to_rally(snap, facts)
    local rally = party.rally(snap, facts)
    if rally == nil then
        return nil
    end
    -- A vanguard on its way to my floor is met where I stand: walking back to the floor it was leaving made a
    -- warrior that crossed first bounce between the two floors until its poet came (offline profile swap seed 2,
    -- 837-841s).
    local lead = party.vanguard(snap, facts)
    if lead ~= nil and lead.world.map ~= snap.world.map and M.on_ground(snap, facts) then
        for _, map in ipairs(lead.heading or {}) do
            if map == snap.world.map then
                return { kind = ACTION.HOLD }
            end
        end
    end
    -- A rally on a member whose moves failed means the crew comes to fetch it. That member still keeps
    -- walking toward the vanguard: one that cannot move fails again and stays fetched, one that can rejoins.
    -- Standing still kept the old failure as the last word, so a member that only waited stayed "stuck"
    -- while the front fought elsewhere (WA+PO 세작, 640-730s). Any other rally on me (the crew comes in
    -- to me already inside) is waited for where I stand.
    if rally.name == snap.self.name then
        local lead = party.vanguard(snap, facts)
        if lead == nil or lead.world.x == nil or party.stuck(snap, snap.pace.swing or data.swing) == false then
            return { kind = ACTION.HOLD }
        end
        rally = { map = lead.world.map, x = lead.world.x, y = lead.world.y }
    end
    -- Arriving is counted in steps, as being together is: four tiles off through a wall that takes 22 steps
    -- round, a rogue counted itself at the rally and stood there while its crew counted it apart, and the whole
    -- trio waited at the rally for good (offline profile trio seed 2, 600-690s).
    if rally.map == snap.world.map and snap.world.x ~= nil then
        local straight = math.abs(rally.x - snap.world.x) + math.abs(rally.y - snap.world.y)
        local steps    = escort.steps(snap, facts, snap, { { x = rally.x, y = rally.y } })[1]
        local walk     = type(steps) == "number" and steps or straight
        if walk <= data.adjacent then
            return { kind = ACTION.HOLD }
        end
        local range = straight <= data.adjacent and 1 or data.adjacent
        return { kind = ACTION.MOVE, dest = { map = rally.map, x = rally.x, y = rally.y, range = range } }
    end
    return { kind = ACTION.MOVE, dest = { maps = { rally.map } } }
end

local function to_center(snap, facts)
    local center = escort.center(snap)
    local reach  = recovery.heal(snap) ~= nil and escort.complete(snap, facts) and
        math.min(data.sight.x, data.sight.y) or data.adjacent
    if center == nil or math.abs(center.x - snap.world.x) + math.abs(center.y - snap.world.y) <= reach then
        return nil
    end
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = center.x, y = center.y, range = reach } }
end

local function beside(snap, facts)
    return M.gather(snap, facts)
end

local function in_way(snap, member, x, y)
    local beside = math.abs(member.world.x - snap.world.x) + math.abs(member.world.y - snap.world.y) == 1
    local mine   = math.abs(x - snap.world.x) + math.abs(y - snap.world.y)
    return beside and mine > 0 and mine < math.abs(x - member.world.x) + math.abs(y - member.world.y)
end

local function off_landing(snap, facts, member)
    local coming = false
    for _, map in ipairs(member.heading or {}) do
        coming = coming or map == snap.world.map
    end
    if coming == false or member.world.map == snap.world.map or member.world.x == nil then
        return nil
    end
    local route = facts.path({ maps = { snap.world.map } }, {}, {},
        { map = member.world.map, x = member.world.x, y = member.world.y, level = member.self.level })
    if route == nil or route.x == nil or math.abs(route.x - snap.world.x) + math.abs(route.y - snap.world.y) > 1 then
        return nil
    end
    local goals = {}
    for dx = -2, 2 do
        local dy = 2 - math.abs(dx)
        table.insert(goals, { x = route.x + dx, y = route.y + dy, range = 0 })
        if dy ~= 0 then
            table.insert(goals, { x = route.x + dx, y = route.y - dy, range = 0 })
        end
    end
    local steps = facts.steps ~= nil and facts.steps(snap.world.map, snap.world.x, snap.world.y, goals) or {}
    local best  = nil
    for index, goal in ipairs(goals) do
        if type(steps[index]) == "number" and (best == nil or steps[index] < best.steps) then
            best = { x = goal.x, y = goal.y, steps = steps[index] }
        end
    end
    return best ~= nil and { kind = ACTION.MOVE, dest = { map = snap.world.map, x = best.x, y = best.y, range = 0 } } or
        nil
end

-- Making way is a step aside onto the nearest tile off the member's way, walked in as many steps as that tile
-- takes. Walking on toward its goal until one tile nearer sent a poet 35 steps round a wall, out of its
-- warrior's sight, when the tiles beside it were taken (offline stage supply seed 2, 158-167s). With no tile
-- aside, as in a corridor one tile wide, I go one step ahead of it toward its goal: waiting for a tile aside
-- left a warrior and its poet facing each other in a corridor for 290s (offline WA+PO 세작 seed 0, 401-691s).
-- A tile aside lies on my side of the member: one past it is reached only through the member's own tile, and
-- a poet walking for it and a warrior behind it stood blocked at a dead end for minutes (offline WA+PO seed 0,
-- 217-500s).
local function aside(snap, facts, member, going)
    local ahead = math.abs(going.x - member.world.x) + math.abs(going.y - member.world.y)
    local mine  = math.abs(going.x - snap.world.x) + math.abs(going.y - snap.world.y)
    local goals = {}
    for dx = -2, 2 do
        for dy = -2, 2 do
            local far    = math.abs(dx) + math.abs(dy)
            local x, y   = snap.world.x + dx, snap.world.y + dy
            local toward = math.abs(going.x - x) + math.abs(going.y - y)
            local off    = toward >= ahead and math.abs(x - member.world.x) + math.abs(y - member.world.y) >= far
            local on     = far == 1 and toward < mine
            if far > 0 and far <= 2 and (x ~= member.world.x or y ~= member.world.y) and (off or on) and
                (facts.floor == nil or facts.floor(snap.world.map, x, y)) then
                table.insert(goals, { x = x, y = y, range = 0, far = far, off = off })
            end
        end
    end
    local steps = facts.steps ~= nil and #goals > 0 and facts.steps(snap.world.map, snap.world.x, snap.world.y, goals) or
        {}
    local best  = nil
    for index, goal in ipairs(goals) do
        local walk = steps[index]
        if type(walk) == "number" and walk <= goal.far and (best == nil or (goal.off and best.off == false) or
                (goal.off == best.off and walk < best.steps)) then
            best = { x = goal.x, y = goal.y, steps = walk, off = goal.off }
        end
    end
    return best ~= nil and
        { kind = ACTION.MOVE, dest = { map = snap.world.map, x = best.x, y = best.y, range = 0, steps = best.steps } } or
        nil
end

local function clear_way(snap, facts)
    for _, member in ipairs(party.members(snap)) do
        local landing = off_landing(snap, facts, member)
        if landing ~= nil then
            return landing
        end
        local going = member.going
        if going ~= nil and member.task == "FIELD" and going.map == snap.world.map and member.world.map == snap.world.map then
            local gap = math.abs(going.x - snap.world.x) + math.abs(going.y - snap.world.y)
            local moved = gap >= data.adjacent and in_way(snap, member, going.x, going.y) and
                aside(snap, facts, member, going) or nil
            -- I am in the member's way only when its walk round me is longer than my step aside and back: on
            -- open floor its walker turns round me at once, and the tile I stepped aside to was the lane it
            -- turned to, so we both switched lanes in step for 90s (offline RO+PO 흉가 seed 0, map 92, 240-330s).
            if moved ~= nil and
                facts.detour(snap.world.map, member.world, going, snap.world) > 2 * moved.dest.steps then
                return moved
            end
        end
    end
    return nil
end

local function along(snap, facts)
    local cleared = clear_way(snap, facts)
    if cleared ~= nil then
        return cleared
    end
    local lead  = party.vanguard(snap, facts)
    local going = lead ~= nil and lead.going or nil
    if going == nil or going.map ~= snap.world.map then
        return nil
    end
    local gap = math.abs(going.x - snap.world.x) + math.abs(going.y - snap.world.y)
    if gap <= data.adjacent then
        return nil
    end
    local leg = math.min(data.sight.x, data.sight.y) - data.adjacent
    return { kind = ACTION.MOVE, dest = { map = going.map, x = going.x, y = going.y, range = math.max(data.adjacent, gap - leg) } }
end

local function wait(_snap, _facts)
    return { kind = ACTION.HOLD }
end

-- Crossing to the next floor is one move of the whole group: when the vanguard standing beside me heads
-- for a floor of the plan, I go with it in the same decision. Before, members waited and only followed
-- after the front had arrived, and the rally that decided that afterwards could send it back
-- (WA+PO, 957/958: the leader crossed and returned again and again).
local function cross_with(snap, facts)
    local lead = party.vanguard(snap, facts)
    if lead == nil or lead.world.map ~= snap.world.map or lead.world.x == nil or lead.heading == nil then
        return nil
    end
    if math.abs(lead.world.x - snap.world.x) + math.abs(lead.world.y - snap.world.y) > 2 * data.adjacent then
        return nil
    end
    local plan = party.plan(snap, facts)
    for _, map in ipairs(lead.heading) do
        if map ~= snap.world.map and floor_of(snap, plan, map) ~= nil then
            return { kind = ACTION.MOVE, dest = { maps = { map } } }
        end
    end
    return nil
end

-- After a walk stopped on the leash I wait where I stopped until a leashed crewmate moves; repeating the
-- same walk at once would stop again before its first step, forever. The wait is published (leashed)
-- so the crew closes in to me (close_in), and then the walk goes on. A hitter beside me ends the wait.
local function leashed_here(snap, facts)
    local leashed = snap.record.memory.leashed
    if party.vanguard_name(snap, facts) ~= snap.self.name or leashed == nil or leashed.map ~= snap.world.map or
        leashed.x ~= snap.world.x or
        leashed.y ~= snap.world.y or
        require("brain.tasks.escape").adjacent_hitter(snap) ~= nil then
        return false
    end
    local count = 0
    for name, at in pairs(leashed.crew or {}) do
        local ally = facts.ally(name)
        if ally == nil or ally.x ~= at.x or ally.y ~= at.y then
            return false
        end
        count = count + 1
    end
    return count > 0
end

local function wait_leashed(_snap, _facts)
    return { kind = ACTION.HOLD, leashed = true }
end

M.wait_leash = when(leashed_here, act("FIELD", wait_leashed, "wait_leashed"), "leash")

local function close_in(snap, facts)
    local lead = party.vanguard(snap, facts)
    if lead == nil or lead.leashed ~= true or lead.world.map ~= snap.world.map or lead.world.x == nil or
        math.abs(lead.world.x - snap.world.x) + math.abs(lead.world.y - snap.world.y) <= data.adjacent then
        return nil
    end
    return { kind = ACTION.MOVE, dest = { map = lead.world.map, x = lead.world.x, y = lead.world.y, range = data.adjacent } }
end

-- Regrouping first meets at the rally point; only a group already on one map tightens toward its centre.
M.regroup = choose({
    act("FIELD", M.to_rally, "rally"),
    act("FIELD", to_center, "to_center"),
}, "regroup")

local function moving(propose)
    return function(snap, facts)
        local action = propose(snap, facts)
        return action ~= nil and action.kind == ACTION.MOVE and action or nil
    end
end

-- Under blows only a walk to the crew comes before the fight: a rally that kept me on my own spot made the
-- vanguard stand and take blows without striking back for 45s (offline stage supply seed 2, 165-210s).
M.rejoin = choose({
    act("FIELD", moving(M.to_rally), "rally"),
    act("FIELD", to_center, "to_center"),
}, "rejoin")

M.keep = when(M.apart, when(M.on_ground, choose({
    act("FIELD", close_in, "close_in"),
    act("FIELD", cross_with, "cross_with"),
    M.regroup,
    act("FIELD", along, "along"),
    act("FIELD", wait, "wait_crew"),
})), "keep_together")

M.follow = choose({
    act("FIELD", close_in, "close_in"),
    M.regroup,
    act("FIELD", wait, "wait_leader"),
}, "follow_crew")

M.make_way = act("FIELD", clear_way, "make_way")

M.advance = choose({
    act("FIELD", close_in, "close_in"),
    act("FIELD", cross_with, "cross_with"),
    act("FIELD", M.to_rally, "rally"),
    act("FIELD", along, "along"),
    act("FIELD", beside, "beside"),
    act("FIELD", wait, "wait_leader"),
}, "advance_crew")

-- The gap between casts walks toward the crew: a healer that stood 6 tiles off while healing never closed
-- in, and fell 50 steps behind when the warrior walked off round a wall (WA+PO 세작, 525-546s). Only the
-- steps that fit the gap are walked, so the next cast is not held back. Beside a mob I stay: leaving it
-- costs the fight its target. Off the ground the entrance is the meeting point that every member walks to
-- on its own: closing in on a warrior just out of its shop sent a poet waiting at the entrance on a 57s
-- walk across 국내성 and back (online group, 16:22).
local function placed(snap, facts, budget)
    if M.grouped(snap) == false or budget <= 0 or snap.world.x == nil or M.on_ground(snap, facts) == false then
        return nil
    end
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            return nil
        end
    end
    local place = spacing.place(snap, facts, budget)
    return place ~= nil and
        { kind = ACTION.MOVE, dest = { map = snap.world.map, x = place.x, y = place.y, range = 0, steps = place.steps } } or
        nil
end

function M.closer(snap, facts, gap)
    return placed(snap, facts, math.floor(gap / (snap.pace.step or data.walk) + 0.5))
end

function M.gather(snap, facts)
    return placed(snap, facts, escort.reach())
end

-- A healer weighs closing the gap against its heals: one action's walk, worth the gap cost it saves. Beside
-- its crew that is little and heals come first; the farther it is the more the walk is worth.
local function stride(snap)
    return math.floor(require("brain.policies.combat").cast(snap) / (snap.pace.step or data.walk) + 0.5)
end

local supports = setmetatable({}, { __mode = "k" })

function M.supporting(snap, facts)
    if supports[snap] == nil then
        supports[snap] = M.grouped(snap) and party.vanguard_name(snap, facts) ~= snap.self.name and
            require("brain.policies.score").capability(snap, facts).hps.others > 0
    end
    return supports[snap]
end

function M.close(snap, facts)
    return placed(snap, facts, stride(snap))
end

function M.close_worth(snap, facts)
    local place = spacing.place(snap, facts, stride(snap))
    return place ~= nil and place.gain or nil
end

function M.can_move(snap)
    local blocked = snap.record.memory.blocked
    return type(blocked) ~= "table" or snap.now - blocked.at >= (snap.pace.swing or data.swing)
end

return M
