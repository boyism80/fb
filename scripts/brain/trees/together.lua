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

function M.walk(snap, facts, member, spot)
    local world = member.world
    local walk  = escort.steps(snap, facts, member, { { x = spot.x, y = spot.y } })[1]
    return type(walk) == "number" and walk or math.abs(spot.x - world.x) + math.abs(spot.y - world.y)
end

-- Waiting at an exit is done in front of it, on the tile that lengthens the ways into it least once the crewmates
-- already standing there keep theirs, and among tiles as good on the nearest: within the steps I wait in (four, so
-- the crew and its vanguard stand within the eight it gathers in), off the tiles whose loss cuts the way (atlas lanes)
-- and off the tiles beside a warp. The ways in are the walks to the exit from the tiles just past those steps.
-- Waiting on any tile within four steps, two crews stood in the way down to 흑해골굴 from 국내성, each holding its own
-- vanguard and the other crew's rogue, for ten minutes (online 170334, 17:05-17:15; user 11-15). Kept off the lanes
-- alone, a rogue and a poet stood side by side across the two-wide way there and held their own vanguard 45s (online
-- 180725, 18:09:36-18:10:21). Far off I walk toward the exit first, as before: picked from anywhere, the tile sent a
-- member walking past its leader to the far side of the door (brain unit test).
function M.waiting(snap, facts, exit)
    local map    = snap.world.map
    local radius = data.adjacent
    if facts.lane == nil or facts.walled == nil or facts.steps == nil or snap.world.x == nil then
        return nil
    end
    local here = M.walk(snap, facts, snap, exit)
    if here > 2 * radius then
        return nil
    end
    local mine  = snap.world.x .. "," .. snap.world.y
    local taken = {}
    local walls = {}
    local function stand(x, y, wall)
        local key = x .. "," .. y
        if key ~= mine and taken[key] == nil then
            taken[key] = true
            if wall then
                table.insert(walls, { x = x, y = y })
            end
        end
    end
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil then
            stand(mob.x, mob.y, false)
        end
    end
    local walking = {}
    for _, member in ipairs(party.members(snap)) do
        local world = member.world
        local going = member.going
        if world.map == map and world.x ~= nil and type(going) == "table" and going.map == map and
            (going.x ~= world.x or going.y ~= world.y) then
            walking[world.x .. "," .. world.y] = true
        end
    end
    for _, tile in ipairs(snap.world.occupied or {}) do
        stand(tile.x, tile.y, walking[tile.x .. "," .. tile.y] == nil)
    end
    for _, member in ipairs(party.members(snap)) do
        if member.world.map == map and member.world.x ~= nil then
            stand(member.world.x, member.world.y, walking[member.world.x .. "," .. member.world.y] == nil)
        end
    end
    local goals = {}
    local ring  = {}
    for dx = -(radius + 1), radius + 1 do
        for dy = -(radius + 1 - math.abs(dx)), radius + 1 - math.abs(dx) do
            local x, y  = exit.x + dx, exit.y + dy
            local open  = x >= 0 and y >= 0 and (facts.floor == nil or facts.floor(map, x, y)) and
                (facts.warp == nil or facts.warp(map, x, y) ~= true)
            local outer = math.abs(dx) + math.abs(dy) == radius + 1
            if open and outer then
                table.insert(ring, { x = x, y = y, range = 0 })
            elseif open and taken[x .. "," .. y] == nil and facts.lane(map, x, y) == false then
                table.insert(goals, { x = x, y = y, range = 0 })
            end
        end
    end
    if #goals == 0 then
        return nil
    end
    local near    = facts.steps(map, exit.x, exit.y, goals)
    local walks   = facts.steps(map, snap.world.x, snap.world.y, goals)
    local choices = {}
    for index, goal in ipairs(goals) do
        local from = near[index]
        local walk = walks[index]
        if type(from) == "number" and from <= radius and type(walk) == "number" then
            table.insert(choices, { x = goal.x, y = goal.y, walk = walk, from = from })
        end
    end
    table.sort(choices, function(a, b)
        return a.walk < b.walk or (a.walk == b.walk and a.from < b.from)
    end)
    local ways = {}
    local base = {}
    for index, steps in ipairs(#ring > 0 and facts.walled(map, exit.x, exit.y, ring, walls) or {}) do
        if type(steps) == "number" then
            table.insert(ways, ring[index])
            table.insert(base, steps)
        end
    end
    local best = nil
    for _, choice in ipairs(choices) do
        local blocked = { { x = choice.x, y = choice.y } }
        for _, wall in ipairs(walls) do
            table.insert(blocked, wall)
        end
        local found = #ways > 0 and facts.walled(map, exit.x, exit.y, ways, blocked) or {}
        local lost  = 0
        for index, steps in ipairs(base) do
            lost = lost + (type(found[index]) == "number" and found[index] - steps or math.huge)
        end
        if best == nil or lost < best.lost then
            best      = choice
            best.lost = lost
        end
        if lost == 0 then
            break
        end
    end
    if best == nil or (best.x == snap.world.x and best.y == snap.world.y) then
        return nil
    end
    return { kind = ACTION.MOVE, dest = { map = map, x = best.x, y = best.y, range = 0 } }
end

-- A member on another floor comes to mine when its way to the floor it heads for ends here or runs through here.
-- Counting only where it heads, a healer that crossed ahead to the floor between saw its vanguard heading past
-- it, walked back to the floor the vanguard was leaving and bounced between the two (offline profile swap seed 1,
-- 618-627s and seed 2, 835-841s). A member that cannot move is not coming, and the crew fetches it (party.pinned).
local function passing(snap, facts, member)
    if member.world.map == snap.world.map or member.world.x == nil or member.heading == nil or member.pinned == true then
        return false
    end
    local through = false
    for _, map in ipairs(member.heading) do
        through = through or map == snap.world.map
    end
    if through == false then
        local route = facts.path({ maps = member.heading }, {}, {},
            { map = member.world.map, x = member.world.x, y = member.world.y, level = member.self.level })
        for _, hop in ipairs(route ~= nil and route.path or {}) do
            through = through or hop.dest == snap.world.map
        end
    end
    return through
end

-- A member the leader sent a crewmate to save stays for it where it stands, and under blows fights there: walking
-- to the rally while its rescuer walked to it, a rogue and its warrior passed each other across the floor in one
-- walk each, and the poet left between them fought alone (offline profile trio rooted seed 1, 507-520s, run 07:05).
local function awaiting(snap, facts)
    local rescue = snap.leader == snap.self.name and party.rescue(snap, facts) or (party.leader(snap) or {}).rescue
    return rescue ~= nil and rescue.name == snap.self.name and rescue.by ~= snap.self.name
end

-- Moving while the group is scattered always goes through the rally point the leader chose
-- (party.rally). This is the only place that decides where scattered members meet; see party.rally
-- for why the per-branch choices it replaced bounced and deadlocked.
function M.to_rally(snap, facts)
    local rally = party.rally(snap, facts)
    if rally == nil then
        return nil
    elseif awaiting(snap, facts) then
        return { kind = ACTION.HOLD }
    end
    -- A vanguard on its way to my floor is met where I stand: walking back to the floor it was leaving made a
    -- warrior that crossed first bounce between the two floors until its poet came (offline profile swap seed 2,
    -- 837-841s).
    local lead = party.vanguard(snap, facts)
    if lead ~= nil and M.on_ground(snap, facts) and passing(snap, facts, lead) then
        return { kind = ACTION.HOLD }
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
    if awaiting(snap, facts) then
        return nil
    end
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

-- How much longer the member's walk to its goal is round me, beside it. Nearer its goal than the member in a straight
-- line is no test of that: a poet south of its warrior, on the warrior's only way round a wall to its exit, stood
-- farther from the exit than the warrior, never made way, and both stood for 360s (offline WA+PO 세작 seed 0,
-- 540-900s, run 17:35).
local function in_way(snap, facts, member, going)
    if math.abs(member.world.x - snap.world.x) + math.abs(member.world.y - snap.world.y) ~= 1 then
        return 0
    end
    return facts.detour(snap.world.map, member.world, going, snap.world)
end

local function front(snap, x, y)
    local nearest = nil
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil then
            local gap = math.abs(mob.x - x) + math.abs(mob.y - y)
            nearest   = nearest == nil and gap or math.min(nearest, gap)
        end
    end
    return nearest or 0
end

-- Off the landing the vanguard steps toward the mobs and the rest no nearer them than the vanguard stands: stepping to
-- the nearest tile, a warrior first on 흑해골굴3 stepped back into a dead end beside the warp home, its rogue after it,
-- and the poet that landed last stood alone in the one-tile neck ahead of them with a skeleton on it (online trio
-- 085344, 18:07; user 11-14: the vanguard always goes in first).
local function landing_tile(snap, facts, goals, steps)
    local leading  = party.vanguard_name(snap, facts) == snap.self.name
    local vanguard = leading == false and party.vanguard(snap, facts) or nil
    local limit    = vanguard ~= nil and vanguard.world.map == snap.world.map and vanguard.world.x ~= nil and
        front(snap, vanguard.world.x, vanguard.world.y) or nil
    for _, bound in ipairs(limit ~= nil and { limit, false } or { false }) do
        local best = nil
        for index, goal in ipairs(goals) do
            local walk  = steps[index]
            local ahead = front(snap, goal.x, goal.y)
            if type(walk) == "number" and (bound == false or ahead >= bound) then
                local rank = leading and ahead or 0
                if best == nil or rank < best.rank or (rank == best.rank and walk < best.steps) then
                    best = { x = goal.x, y = goal.y, steps = walk, rank = rank }
                end
            end
        end
        if best ~= nil then
            return best
        end
    end
    return nil
end

local function off_landing(snap, facts, member)
    if passing(snap, facts, member) == false then
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
    local best  = landing_tile(snap, facts, goals, steps)
    return best ~= nil and { kind = ACTION.MOVE, dest = { map = snap.world.map, x = best.x, y = best.y, range = 0 } } or
        nil
end

-- Making way is a step aside onto the nearest tile off the member's way, walked in as many steps as that tile
-- takes. Walking on toward its goal until one tile nearer sent a poet 35 steps round a wall, out of its
-- warrior's sight, when the tiles beside it were taken (offline stage supply seed 2, 158-167s). With no tile
-- aside, as in a corridor one tile wide, I go one step ahead of it toward its goal: waiting for a tile aside
-- left a warrior and its poet facing each other in a corridor for 290s (offline WA+PO 세작 seed 0, 401-691s). The
-- step ahead stops short of the goal itself: stepping onto it, a warrior took the tile its poet was walking to
-- (offline profile 63+65 form seed 0, 735s, run 20:51).
-- A tile aside lies on my side of the member: one past it is reached only through the member's own tile, and
-- a poet walking for it and a warrior behind it stood blocked at a dead end for minutes (offline WA+PO seed 0,
-- 217-500s). Off the way is a tile the member's walk to its goal does not go round, counted in steps: counted as no
-- nearer its goal in a straight line, a poet stepped from its warrior's only way round a wall onto the next tile of
-- that way (offline WA+PO 세작 seed 0, 540-900s, run 17:35).
-- A tile a mob or a crewmate stands on is no tile to step aside to: chosen anyway, a poet under blows walked at the
-- skeleton beside it to clear its warrior's way, was struck on the way and tried again, 237 times in a run (offline
-- profile 63+65 seed 0, run 02:47).
-- No tile aside takes me farther by walk from where the member goes than I stand: the nearest was a pocket beside a
-- one-tile neck, farther from the entrance we both walked to, so my own walk to the entrance took me back into the
-- neck at once and its leader turned back from it, both for 470s (offline watch mages seed 0, map 3176, 244-720s,
-- run 18:31). Within the steps a member waits at from where it goes, my walk keeps me where I stepped, so a tile
-- there is no farther: counted so, a poet beside the stairs of 국내성, every tile off them a step farther, never made
-- way for its warrior (brain unit test, online WA+PO+MA 150824).
-- A warp is no tile to stand aside on: it takes me off the map, and a poet making way at the stairs of 국내성 chose
-- the second tile into 흑해골굴 and would have gone in ahead of its warrior (brain unit test, online WA+PO+MA 150824).
local function aside(snap, facts, member, going)
    local mine  = math.abs(going.x - snap.world.x) + math.abs(going.y - snap.world.y)
    local goals = {}
    local taken = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil then
            taken[mob.x .. "," .. mob.y] = true
        end
    end
    for _, other in ipairs(party.members(snap)) do
        if other.world.map == snap.world.map and other.world.x ~= nil then
            taken[other.world.x .. "," .. other.world.y] = true
        end
    end
    for dx = -2, 2 do
        for dy = -2, 2 do
            local far  = math.abs(dx) + math.abs(dy)
            local x, y = snap.world.x + dx, snap.world.y + dy
            local side = math.abs(x - member.world.x) + math.abs(y - member.world.y) >= far
            local on   = far == 1 and math.abs(going.x - x) + math.abs(going.y - y) < mine and
                (x ~= going.x or y ~= going.y)
            if far > 0 and far <= 2 and taken[x .. "," .. y] == nil and (side or on) and
                (facts.floor == nil or facts.floor(snap.world.map, x, y)) and
                (facts.warp == nil or facts.warp(snap.world.map, x, y) ~= true) then
                table.insert(goals, { x = x, y = y, range = 0, far = far, side = side, on = on })
            end
        end
    end
    local steps  = {}
    local toward = {}
    if facts.steps ~= nil and #goals > 0 then
        local points = { table.unpack(goals) }
        table.insert(points, { x = snap.world.x, y = snap.world.y })
        steps  = facts.steps(snap.world.map, snap.world.x, snap.world.y, goals)
        toward = facts.steps(snap.world.map, going.x, going.y, points)
    end
    local here = type(toward[#goals + 1]) == "number" and toward[#goals + 1] or math.huge
    local best = nil
    for index, goal in ipairs(goals) do
        local walk  = steps[index]
        local near  = type(walk) == "number" and walk <= goal.far
        local off   = near and goal.side and facts.detour(snap.world.map, member.world, going, goal) == 0
        local ahead = type(toward[index]) == "number" and toward[index] or math.huge
        if near and (off or goal.on) and ahead <= math.max(here, data.adjacent) and (best == nil or (off and best.off == false) or
                (off == best.off and walk < best.steps)) then
            best = { x = goal.x, y = goal.y, steps = walk, off = off }
        end
    end
    return best ~= nil and
        { kind = ACTION.MOVE, dest = { map = snap.world.map, x = best.x, y = best.y, range = 0, steps = best.steps } } or
        nil
end

-- A crewmate walking at the mob it fights is going there as much as one walking to a tile, and it walks there when
-- its strongest blow is struck beside the mob. Made way for on its walks alone, a poet held its tile in a corridor
-- one tile wide while its rogue swung at the skeletons past it, and every swing found no way (offline RO+PO 세작
-- seed 0, 60-900s, 878 swings unreachable, run 15:23); made way for whatever it fought, a warrior stepped aside for
-- the mob its poet cursed from afar and walked off round the floor (offline WA+PO 세작 seed 0, 409-410s, run 16:24).
local function heading(snap, facts, member)
    local fight = member.battle
    if member.going ~= nil or type(fight) ~= "table" or fight.map ~= snap.world.map then
        return member.going
    end
    if require("brain.policies.score").melee(member, facts) == false then
        return nil
    end
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.oid == fight.oid and mob.x ~= nil and
            math.abs(mob.x - member.world.x) + math.abs(mob.y - member.world.y) > 1 then
            return { map = snap.world.map, x = mob.x, y = mob.y }
        end
    end
    return nil
end

local function clear_way(snap, facts)
    for _, member in ipairs(party.members(snap)) do
        local landing = off_landing(snap, facts, member)
        if landing ~= nil then
            return landing
        end
        local going = heading(snap, facts, member)
        if going ~= nil and member.task == "FIELD" and going.map == snap.world.map and member.world.map == snap.world.map then
            local gap     = math.abs(going.x - snap.world.x) + math.abs(going.y - snap.world.y)
            local stopped = member.blocker ~= nil and member.blocker.x == snap.world.x and
                member.blocker.y == snap.world.y
            local round   = stopped and math.huge or gap >= data.adjacent and in_way(snap, facts, member, going) or 0
            local moved   = round > 0 and aside(snap, facts, member, going) or nil
            -- I am in the member's way only when its walk round me is longer than my step aside and back: on
            -- open floor its walker turns round me at once, and the tile I stepped aside to was the lane it
            -- turned to, so we both switched lanes in step for 90s (offline RO+PO 흉가 seed 0, map 92, 240-330s).
            if moved ~= nil and round > 2 * moved.dest.steps then
                return moved
            end
        end
    end
    return nil
end

-- A member walks to where the vanguard stands now, not to where it is going: heading for the vanguard's goal, a
-- crewmate took another way at a fork and the group split (offline trio scene on 흉가 91). Near is counted in
-- walking steps: a poet 4 tiles from its waiting rogue but 12 steps round a wall counted itself close, and both
-- waited (offline RO+PO 흉가 seed 2, 495-516s, run 10:40). While the vanguard moves I walk a short leg and look
-- again, so I head for where it is then.
local function to_vanguard(snap, facts, lead, budget)
    local ally = facts.ally(lead.self.name)
    local spot = ally ~= nil and ally.x ~= nil and ally or lead.world
    local walk = escort.steps(snap, facts, snap, { { x = spot.x, y = spot.y } })[1]
    if type(walk) ~= "number" or walk <= data.adjacent then
        return nil
    end
    return {
        kind = ACTION.MOVE,
        dest = {
            map = snap.world.map,
            x = spot.x,
            y = spot.y,
            range = 1,
            steps = math.min(budget, walk - data.adjacent)
        }
    }
end

local function along(snap, facts)
    local cleared = clear_way(snap, facts)
    if cleared ~= nil then
        return cleared
    end
    local lead = party.vanguard(snap, facts)
    if lead == nil or lead.going == nil or lead.going.map ~= snap.world.map or lead.world.map ~= snap.world.map or
        lead.world.x == nil then
        return nil
    end
    return to_vanguard(snap, facts, lead, escort.reach() - data.adjacent)
end

local function waiting_lead(snap, facts)
    local lead = party.vanguard(snap, facts)
    if lead == nil or lead.leashed ~= true or lead.world.map ~= snap.world.map or lead.world.x == nil then
        return nil
    end
    return lead
end

local function wait(_snap, _facts)
    return { kind = ACTION.HOLD }
end

-- A member that has not moved since its walk failed walks it again instead of waiting for its crew: the crew waits
-- for a pinned member as long as it is pinned, so a poet rooted beside its warrior that kept waiting for the
-- warrior to cross would have stood with it for good once free (offline profile pair rooted healer seed 2). The
-- failure backoff paces the tries, and the first that goes through ends the pin.
local function retry(snap, _facts)
    local blocked = snap.record.memory.blocked
    if party.pinned(snap) == false or blocked.dest == nil then
        return nil
    end
    return { kind = ACTION.MOVE, dest = blocked.dest }
end

-- Crossing to the next floor the vanguard goes first: when the vanguard standing beside me heads for a floor of the
-- plan, I stay beside it until it is over, and the rally takes me after it. Crossing in the same decision, a rogue and
-- a poet landed on the floor with their warrior and the poet, last, stood ahead of it (online trio 085344, 18:07;
-- user 11-14). Beside is counted in my steps to it, as it counts me gathered at the exit (hunt.stage).
local function cross_after(snap, facts)
    local lead = party.vanguard(snap, facts)
    if lead == nil or lead.world.map ~= snap.world.map or lead.world.x == nil or lead.heading == nil then
        return nil
    end
    if M.walk(snap, facts, snap, lead.world) > 2 * data.adjacent then
        return nil
    end
    local plan = party.plan(snap, facts)
    for _, map in ipairs(lead.heading) do
        if map ~= snap.world.map and floor_of(snap, plan, map) ~= nil then
            return { kind = ACTION.HOLD }
        end
    end
    return nil
end

local function close_in(snap, facts)
    local lead = waiting_lead(snap, facts)
    return lead ~= nil and to_vanguard(snap, facts, lead, escort.reach() - data.adjacent) or nil
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
    act("FIELD", cross_after, "cross_after"),
    M.regroup,
    act("FIELD", along, "along"),
    act("FIELD", retry, "retry_walk"),
    act("FIELD", wait, "wait_crew"),
})), "keep_together")

M.follow = choose({
    act("FIELD", close_in, "close_in"),
    M.regroup,
    act("FIELD", retry, "retry_walk"),
    act("FIELD", wait, "wait_leader"),
}, "follow_crew")

M.make_way = act("FIELD", clear_way, "make_way")

M.advance = choose({
    act("FIELD", close_in, "close_in"),
    act("FIELD", cross_after, "cross_after"),
    act("FIELD", M.to_rally, "rally"),
    act("FIELD", along, "along"),
    act("FIELD", beside, "beside"),
    act("FIELD", retry, "retry_walk"),
    act("FIELD", wait, "wait_leader"),
}, "advance_crew")

-- The gap between casts walks toward the crew: a healer that stood 6 tiles off while healing never closed
-- in, and fell 50 steps behind when the warrior walked off round a wall (WA+PO 세작, 525-546s). Only the
-- steps that fit the gap are walked, so the next cast is not held back. Off the ground the entrance is the
-- meeting point that every member walks to on its own: closing in on a warrior just out of its shop sent a poet
-- waiting at the entrance on a 57s walk across 국내성 and back (online group, 16:22). While the vanguard waits for
-- the crew my place is beside it: placing myself between my crewmates walked a poet and a rogue round each other,
-- each out of the waiting warrior's walking reach in turn, and the trio stood 100s on 흉가 91 (unit: trio moves to
-- the next floor). Beside a mob I stay for that walk, as leaving it costs the fight its target. Elsewhere the tile
-- is weighed with the blows taken there and the swings given up off a mob's side (spacing.place). A bot alone has
-- no entrance to meet at and takes its tile wherever it fights.
local function placed(snap, facts, budget)
    if budget <= 0 or snap.world.x == nil or (M.grouped(snap) and M.on_ground(snap, facts) == false) then
        return nil
    end
    local lead = waiting_lead(snap, facts)
    if lead == nil then
        local place = spacing.place(snap, facts, budget)
        return place ~= nil and
            { kind = ACTION.MOVE, dest = { map = snap.world.map, x = place.x, y = place.y, range = 0, steps = place.steps } } or
            nil
    else
        local engaged = false
        for _, mob in ipairs(snap.world.mobs or {}) do
            engaged = engaged or (mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1)
        end
        return engaged == false and to_vanguard(snap, facts, lead, budget) or nil
    end
end

function M.closer(snap, facts, gap)
    return placed(snap, facts, math.floor(gap / (snap.pace.step or data.walk) + 0.5))
end

function M.gather(snap, facts)
    return placed(snap, facts, escort.reach())
end

-- A healer weighs closing the gap against its heals: one action's walk, worth the gap cost it saves. Beside
-- its crew that is little and heals come first; the farther it is the more the walk is worth. While the vanguard
-- waits for the crew the walk is to the vanguard (close_in), not to a place among the crew.
local function stride(snap)
    return math.floor(require("brain.policies.combat").cast(snap) / (snap.pace.step or data.walk) + 0.5)
end

local supports = setmetatable({}, { __mode = "k" })

function M.supporting(snap, facts)
    if supports[snap] == nil then
        supports[snap] = M.grouped(snap) and party.vanguard_name(snap, facts) ~= snap.self.name and
            waiting_lead(snap, facts) == nil and require("brain.policies.score").healer(snap, facts)
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

-- Under blows a bot that strikes from afar steps off and strikes from the new tile, so the step is worth the strike it
-- puts off by its walk, the blows it spares and the death those blows risk, as a heal or a 마비 weighs that death
-- (aid.risk): weighed by the blows alone beside the whole strike, a mage cast half its spells beside the mobs, 137 of
-- 279 (online WA+PO+MA 150824; user 11-15: mages fight keeping their distance first). A bot that strikes beside the
-- mob gives up its strike by stepping, and weighs the step as before.
function M.step_worth(snap, facts)
    local place = spacing.place(snap, facts, stride(snap))
    if place == nil then
        return nil
    end
    if require("brain.policies.score").melee(snap, facts) then
        return place.gain
    end
    local aid    = require("brain.tasks.aid")
    local worth  = require("brain.policies.value").baseline(snap, facts)
    local strike = require("brain.tasks.defend").value(snap, facts)
    local moved  = setmetatable({ world = { map = snap.world.map, x = place.x, y = place.y } }, { __index = snap })
    local spared = aid.risk(snap, snap, snap.self.hp) - aid.risk(snap, moved, snap.self.hp)
    return strike + place.gain + math.max(0, spared) * party.death(snap, facts, snap, worth) -
        place.steps * (snap.pace.step or data.walk) / 1000 * worth.value
end

-- Under blows a step to a better tile is one more answer beside heals and strikes, for any crewmate whose place
-- is not the waiting vanguard's side (user 11-12: poets and mages keep off the mobs and keep the crew's order).
function M.step_off(snap, facts)
    if waiting_lead(snap, facts) ~= nil then
        return nil
    end
    return placed(snap, facts, stride(snap))
end

function M.can_move(snap)
    local blocked = snap.record.memory.blocked
    return type(blocked) ~= "table" or snap.now - blocked.at >= (snap.pace.swing or data.swing)
end

return M
