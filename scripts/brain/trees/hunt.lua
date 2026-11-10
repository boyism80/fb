local data     = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local party    = require("brain.policies.party")
local tree     = require("brain.tree")
local aid      = require("brain.tasks.aid")
local field    = require("brain.tasks.field")
local rest     = require("brain.tasks.rest")
local recovery = require("brain.policies.recovery")
local together = require("brain.trees.together")

local choose, when, act = tree.select, tree.when, tree.act

local function position(snap, floors)
    for index, floor in ipairs(floors) do
        if floor.map == snap.world.map then
            return index
        end
    end
    return nil
end

local function occupy(snap, plan)
    local floor = plan.floors[position(snap, plan.floors)]
    return {
        map = floor.map,
        rate = floor.kills,
        load = (floor.kills or 0) / (#(snap.party or {}) + 1),
        exp = floor.rate,
        deaths = floor.deaths,
        margin = floor.margin,
        wear = floor.wear,
        extra = floor.crowd,
        name = plan.name,
        crew = #(snap.party or {})
    }
end

local function claimed(propose)
    return function(snap, facts)
        local action = propose(snap, facts)
        if action ~= nil then
            local plan  = party.plan(snap, facts)
            local floor = plan.floors[1]
            if floor.kills ~= nil then
                action.occupy = {
                    map = floor.map,
                    rate = floor.kills,
                    load = floor.kills / (#(snap.party or {}) + 1),
                    name =
                        plan.name
                }
            end
        end
        return action
    end
end

local function occupied(propose)
    return function(snap, facts)
        local action = propose(snap, facts)
        if action ~= nil then
            action.occupy = occupy(snap, party.plan(snap, facts))
        end
        return action
    end
end

local function stride(snap, x, y, range)
    local leg = math.min(data.sight.x, data.sight.y) - data.adjacent
    return math.max(range, math.abs(x - snap.world.x) + math.abs(y - snap.world.y) - leg)
end

local function stage(snap, facts, maps, hunting)
    local route = facts.route(maps)
    local exit  = route ~= nil and route.path ~= nil and route.path[1] or nil
    if exit == nil or exit.x == nil then
        return { kind = ACTION.MOVE, dest = { maps = maps } }
    end
    if math.abs(exit.x - snap.world.x) + math.abs(exit.y - snap.world.y) > data.adjacent then
        return {
            kind = ACTION.MOVE,
            dest = {
                map = snap.world.map,
                x = exit.x,
                y = exit.y,
                range = hunting and stride(snap, exit.x, exit.y, data.adjacent) or data.adjacent
            }
        }
    end
    -- At the exit the group crosses together; a member busy with something else on this spot is waited
    -- for. Entering from outside, the exit is the meeting point: every member walks here on its own after
    -- its own servicing, so the leader waits instead of walking off to fetch anyone. Only a member that
    -- cannot come (moves keep failing, or it is fighting) is fetched, or the wait would never end. A
    -- member already inside the ground is not waited for; the leader goes in to it. A member on the floor I
    -- am leaving is not inside yet: counted so, a vanguard crossed to the next floor without a rogue that was
    -- rooted on its own floor, and a rogue went on and came back for a poet held at the exit (offline profile
    -- trio rooted seed 2, 244-250s; RO+PO 흉가 seed 1, 219-222s).
    local plan = party.plan(snap, facts)
    for _, member in ipairs(party.members(snap)) do
        local world   = member.world
        local through = false
        for _, map in ipairs(maps) do
            through = through or map == world.map
        end
        for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
            through = through or (floor.map == world.map and world.map ~= snap.world.map)
        end
        if member.self.ghost == false and through == false and (world.map ~= snap.world.map or world.x == nil or
                member.pinned or (member.task ~= "FIELD" and member.task ~= "NOTHING") or
                math.abs(snap.world.x - world.x) + math.abs(snap.world.y - world.y) > 2 * data.adjacent) then
            local fetch = hunting == true or member.pinned or member.task == "DEFEND"
            return (fetch and together.to_rally(snap, facts)) or { kind = ACTION.HOLD }
        end
    end
    return { kind = ACTION.MOVE, dest = { maps = maps } }
end

local function leader(snap)
    return snap.leader == snap.self.name
end

local function vanguard(snap, facts)
    return party.vanguard_name(snap, facts) == snap.self.name
end

local function not_vanguard(snap, facts)
    return vanguard(snap, facts) == false
end

-- Entering from outside always works the same way: the group decided the ground, every member walks to
-- its entrance on its own (after its own servicing, or after a flight), and they go in together from
-- there. Walking with crew or fetching by a rally pulled the leader back to wherever a member was, and
-- after a flight let the fleer walk back in alone.
local function lead_gate(snap, facts)
    local plan = party.plan(snap, facts)
    local gate = party.entry_gate(snap, facts, plan)
    if snap.world.map ~= gate then
        return { kind = ACTION.MOVE, dest = { maps = { gate } } }
    end
    return stage(snap, facts, { plan.floors[1].map })
end

local function doorway(facts, floor, map, from)
    local route = from ~= nil and facts.path({ maps = { floor } }, nil, nil, from) or facts.route({ floor })
    local found = nil
    for _, hop in ipairs(route ~= nil and route.path or {}) do
        if hop.dest == floor and hop.map == map then
            found = hop
        end
    end
    return found
end

local function follow_entry(snap, facts)
    local plan = party.plan(snap, facts)
    local lead = party.leader(snap)
    local gate = plan.gate or plan.floors[1].map
    if lead == nil or lead.world.x == nil then
        return { kind = ACTION.HOLD }
    end
    -- While the leader is outside the ground a member never goes in on its own: it walks to the entrance of
    -- the gate map, waits there, and follows only when the leader crosses from that gate. Following the
    -- leader around made the fleer re-enter alone and flee again and again (trio rooted, 777-870s), and
    -- walking to wherever the leader stood on the gate map sent a warrior already at the entrance across
    -- 국내성 after a leader that had just left it (online group, 14:44:55); the leader waits at the same
    -- entrance (stage), so both meet there whatever either did on the way. With the leader already inside,
    -- the member goes to the leader, except right after a flight: the leader inside is about to leave too
    -- (party.regrouping).
    if ground.floor_at(lead.world.map) == nil or party.regrouping(snap) then
        local regate   = party.entry_gate(snap, facts, plan)
        local entering = false
        for _, map in ipairs(lead.heading or {}) do
            entering = entering or map == plan.floors[1].map
        end
        local crossing = snap.world.map == regate and lead.world.map == regate and entering
        if crossing == false then
            if snap.world.map ~= regate then
                return { kind = ACTION.MOVE, dest = { maps = { regate } } }
            end
            local from = lead.world.map == regate and
                { map = lead.world.map, x = lead.world.x, y = lead.world.y, level = lead.self.level } or nil
            local door = doorway(facts, plan.floors[1].map, regate, from)
            if door == nil then
                if lead.world.map ~= regate or
                    math.abs(lead.world.x - snap.world.x) + math.abs(lead.world.y - snap.world.y) <= data.adjacent then
                    return { kind = ACTION.HOLD }
                end
                return { kind = ACTION.MOVE, dest = { map = regate, x = lead.world.x, y = lead.world.y, range = data.adjacent } }
            end
            if math.abs(door.x - snap.world.x) + math.abs(door.y - snap.world.y) <= data.adjacent then
                return { kind = ACTION.HOLD }
            end
            return { kind = ACTION.MOVE, dest = { map = regate, x = door.x, y = door.y, range = data.adjacent } }
        end
    else
        -- While the group is scattered the member goes where the leader's rally point says, not to wherever
        -- the leader happens to be: walking to the leader crossed the leader coming to fetch it.
        local rallied = together.to_rally(snap, facts)
        if rallied ~= nil then
            return rallied
        end
    end
    if lead.world.map ~= snap.world.map and lead.heading == nil then
        local reachable = facts.route({ lead.world.map }) ~= nil
        return { kind = ACTION.MOVE, dest = { maps = { reachable and lead.world.map or plan.floors[1].map } } }
    end
    local arrived = lead.heading == nil
    for _, map in ipairs(lead.heading or {}) do
        arrived = arrived or map == snap.world.map
    end
    if arrived == false then
        return { kind = ACTION.MOVE, dest = { maps = lead.heading } }
    end
    if lead.world.map ~= snap.world.map then
        return { kind = ACTION.HOLD }
    end
    -- Follow to where the leader is going, not to where it stood: on a large map a walk to the leader's
    -- old tile took over a minute and ended far from the exit the leader was waiting at.
    local going = lead.going ~= nil and lead.going.map == snap.world.map and lead.going or lead.world
    if math.abs(going.x - snap.world.x) + math.abs(going.y - snap.world.y) <= data.adjacent then
        return { kind = ACTION.HOLD }
    end
    return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = going.x, y = going.y, range = data.adjacent } }
end

local function solo_entry(snap, facts)
    local plan = party.plan(snap, facts)
    local leg  = snap.record.memory.leg
    local aim  = plan.floors[1].map
    for _, entry in ipairs(leg ~= nil and leg.name == plan.name and plan.floors or {}) do
        if entry.map == leg.map then
            aim = leg.map
        end
    end
    return { kind = ACTION.MOVE, dest = { maps = { aim } }, aim = plan.name }
end

local function short_of_mana(snap, facts)
    return rest.outside(snap, facts) and (snap.self.mp or 0) < rest.mana_need(snap, facts)
end

local function top_up(snap, facts)
    local charge = recovery.spells(snap).charge
    if charge ~= nil and combat.can_cast(snap) and (snap.self.mp or 0) >= charge.mp then
        return { kind = ACTION.CAST, slot = charge.slot }
    end
    local doses = recovery.doses(snap, "mp")
    if #doses > 0 then
        return { kind = ACTION.USE, item = rest.dose(doses, rest.mana_need(snap, facts) - (snap.self.mp or 0)) }
    end
    return nil
end

local enter = choose({
    when(short_of_mana, act("REST", top_up, "top_up")),
    when(function(snap)
        return together.grouped(snap) == false
    end, act("FIELD", claimed(solo_entry), "solo_entry")),
    when(leader, act("FIELD", claimed(lead_gate), "lead_gate")),
    act("FIELD", claimed(follow_entry), "follow_entry"),
}, "enter_ground")

-- The crewmate the leader sent me to save comes before my own prey: the mob in my way to it first, then the
-- ones on it (party.rescue_share), and with none in sight I walk to it.
local function rescue(snap, facts)
    local victim = together.on_ground(snap, facts) and party.rescued(snap, facts) or nil
    if victim == nil or victim.world.map ~= snap.world.map or victim.world.x == nil or
        math.abs(victim.world.x - snap.world.x) + math.abs(victim.world.y - snap.world.y) <= 1 then
        return nil
    end
    local found = battle.target(snap, facts, function(mob)
        return party.rescue_share(snap, facts, mob) > 0 and field.castable(snap, facts, mob)
    end)
    if found == nil then
        return { kind = ACTION.MOVE, dest = { map = victim.world.map, x = victim.world.x, y = victim.world.y, range = 1 } }
    end
    local range = stride(snap, found.mob.x, found.mob.y, 1)
    if found.choice.approach and range > 1 then
        return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = found.mob.x, y = found.mob.y, range = range } }
    end
    return battle.propose(found)
end

local function free_member(snap, facts)
    for _, member in ipairs(party.members(snap)) do
        local world = member.world
        if member.pinned and member.self.ghost == false and world.map == snap.world.map and world.x ~= nil then
            local found = battle.target(snap, facts, function(mob)
                return math.abs(mob.x - world.x) + math.abs(mob.y - world.y) <= 1 and combat.hits(mob) and
                    field.castable(snap, facts, mob)
            end)
            if found ~= nil then
                return battle.propose(found)
            end
        end
    end
    return nil
end

local function strike(snap, facts)
    local plan  = party.plan(snap, facts)
    local found = field.target(snap, facts, plan.floors[position(snap, plan.floors)].targets)
    if found == nil then
        return nil
    end
    -- Beside a crewmate that turns hp into damage, a strike is worth its share of the kill less what its mana
    -- would heal that crewmate, and below that I keep with the crew instead of swinging. Counted free, as a
    -- spell is beside a plain swing, a poet cast 화염주 at full mana and then had to charge for the warrior's
    -- heal (online group, 16:23, 14 of 66 화염주). A vanguard that holds back still has prey here for its crew,
    -- so it stays instead of moving on: falling through to the next floor, the one in front left its warrior
    -- fighting on 국내해골굴1 (online group, 16:47).
    local fuel = aid.fueling(snap, facts, found.mob)
    if battle.marginal(snap, found) < (found.choice.mp or 0) * fuel then
        return vanguard(snap, facts) and { kind = ACTION.HOLD } or nil
    else
        local range = stride(snap, found.mob.x, found.mob.y, 1)
        if found.choice.approach and range > 1 then
            return { kind = ACTION.MOVE, dest = { map = snap.world.map, x = found.mob.x, y = found.mob.y, range = range } }
        end
        return battle.propose(found)
    end
end

local function several(snap, facts)
    return #party.plan(snap, facts).floors > 1
end

local function crew_busy_here(snap)
    for _, member in ipairs(party.members(snap)) do
        if member.world.map == snap.world.map and member.self.ghost == false and member.task ~= nil and
            member.task ~= "FIELD" and member.task ~= "NOTHING" then
            return true
        end
    end
    return false
end

local function left_floors(snap, leg)
    local left = {}
    for map, at in pairs(leg ~= nil and leg.left or {}) do
        left[map] = at
    end
    left[snap.world.map] = snap.now
    return left
end

local function regrown(snap, floor)
    local leg = snap.record.memory.leg
    return leg == nil or leg.left == nil or leg.left[floor.map] == nil
end

local function other_floor_ready(snap, facts)
    local plan = party.plan(snap, facts)
    for _, floor in ipairs(plan.floors) do
        if floor.map ~= snap.world.map and regrown(snap, floor) then
            return true
        end
    end
    return false
end

local function next_floor(snap, facts)
    local plan = party.plan(snap, facts)
    local nxt  = plan.floors[position(snap, plan.floors) % #plan.floors + 1]
    local leg  = snap.record.memory.leg
    for offset = 1, #plan.floors do
        local floor = plan.floors[(position(snap, plan.floors) + offset - 1) % #plan.floors + 1]
        if floor.map ~= snap.world.map and regrown(snap, floor) then
            nxt = floor
            break
        end
    end
    if leg ~= nil and leg.name == plan.name and leg.map ~= snap.world.map then
        for _, entry in ipairs(plan.floors) do
            if entry.map == leg.map then
                nxt = entry
            end
        end
    end
    local action = stage(snap, facts, { nxt.map }, true)
    action.leg = { name = plan.name, map = nxt.map, left = left_floors(snap, leg) }
    return action
end

local function nearest(snap, list)
    local range = math.min(data.sight.x, data.sight.y)
    local found = nil
    local best  = nil
    for _, entry in ipairs(list) do
        local cost = math.abs(entry.x - snap.world.x) + math.abs(entry.y - snap.world.y)
        if cost > range and (best == nil or cost < best) then
            found = entry
            best  = cost
        end
    end
    return found
end

local function roam_to(snap, spot)
    return {
        kind = ACTION.MOVE,
        dest = {
            map = snap.world.map,
            x = spot.x,
            y = spot.y,
            range = stride(snap, spot.x, spot.y, math.min(data.sight.x, data.sight.y))
        },
    }
end

local function waypoints(snap, facts)
    local plan   = party.plan(snap, facts)
    local width  = 2 * data.sight.x + 1
    local height = 2 * data.sight.y + 1
    local found  = {}
    local known  = {}
    for _, area in ipairs(facts.spawns(snap.world.map, plan.floors[position(snap, plan.floors)].targets) or {}) do
        for y = area.top + data.sight.y, area.bottom + data.sight.y, height do
            for x = area.left + data.sight.x, area.right + data.sight.x, width do
                local spot = { x = math.min(x, area.right), y = math.min(y, area.bottom) }
                spot.key   = spot.x .. "," .. spot.y
                if known[spot.key] == nil then
                    known[spot.key] = true
                    table.insert(found, spot)
                end
            end
        end
    end
    return found
end

local function unseen(snap, facts)
    local roam  = snap.record.memory.roam
    local keys  = roam ~= nil and roam.map == snap.world.map and roam.keys or {}
    local list  = {}
    local goals = {}
    for _, spot in ipairs(waypoints(snap, facts)) do
        if keys[spot.key] == nil then
            table.insert(list, spot)
            table.insert(goals, { x = spot.x, y = spot.y, range = math.min(data.sight.x, data.sight.y) })
        end
    end
    local paths = #goals > 0 and facts.steps ~= nil and facts.steps(snap.world.map, snap.world.x, snap.world.y, goals) or
        {}
    local open  = {}
    for index, spot in ipairs(list) do
        if paths[index] ~= false then
            table.insert(open, spot)
        end
    end
    return open
end

local function spawn_area(snap, facts)
    local list    = unseen(snap, facts)
    local viewed  = {}
    local best    = nil
    local closest = nil
    for _, spot in ipairs(list) do
        local dx = math.abs(spot.x - snap.world.x)
        local dy = math.abs(spot.y - snap.world.y)
        if dx <= data.sight.x and dy <= data.sight.y then
            table.insert(viewed, spot.key)
        elseif closest == nil or dx + dy < closest then
            best    = spot
            closest = dx + dy
        end
    end
    if best == nil then
        return #viewed > 0 and { kind = ACTION.HOLD, viewed = viewed } or nil
    end
    local action  = roam_to(snap, best)
    action.viewed = viewed
    return action
end

local function searching(propose)
    return function(snap, facts)
        local action = propose(snap, facts)
        if action ~= nil then
            action.searching = true
        end
        return action
    end
end

local function lap_finished(snap, facts)
    local search = snap.record.memory.search
    if search == nil or search.map ~= snap.world.map or other_floor_ready(snap, facts) then
        return false
    end
    local plan  = party.plan(snap, facts)
    local floor = plan.floors[position(snap, plan.floors)]
    if (floor.density or 0) <= 0 or (floor.mobs or 0) <= 0 then
        return false
    end
    local swath = 2 * data.sight.y + 1
    return #unseen(snap, facts) == 0 or
        snap.now - search.from >= floor.mobs / floor.density / swath * (snap.pace.step or data.walk)
end

local function finish_lap(snap, facts)
    local plan = party.plan(snap, facts)
    local maps = {}
    for _, floor in ipairs(plan.floors) do
        table.insert(maps, floor.map)
    end
    return { kind = ACTION.HOLD, revisit = 1, lap_done = { name = plan.name, maps = maps } }
end

local function seen_prey(snap, facts)
    local plan   = party.plan(snap, facts)
    local wanted = {}
    for _, name in ipairs(plan.floors[position(snap, plan.floors)].targets) do
        wanted[name] = true
    end
    local list = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and wanted[mob.name] ~= nil then
            table.insert(list, mob)
        end
    end
    local spot = nearest(snap, list)
    return spot ~= nil and roam_to(snap, spot) or nil
end

local function idle(_snap, _facts)
    return { kind = ACTION.HOLD }
end

local on_ground = choose({
    act("FIELD", occupied(free_member), "free_member"),
    act("FIELD", occupied(strike), "strike"),
    when(several, choose({
        when(together.grouped, choose({
            when(not_vanguard, together.advance),
            when(crew_busy_here, together.advance),
        })),
        when(other_floor_ready, act("FIELD", occupied(next_floor), "next_floor")),
    })),
    when(together.grouped, when(not_vanguard, together.follow)),
    when(lap_finished, act("FIELD", finish_lap, "lap_done")),
    act("FIELD", searching(occupied(spawn_area)), "spawn_area"),
    act("FIELD", searching(occupied(seen_prey)), "seen_prey"),
    act("FIELD", searching(idle), "no_prey"),
}, "on_ground")

-- Servicing is decided for the whole group (decide.lap): a member still on a hunting floor first walks
-- out to the entrance, then services on its own outside. Staying on the floor kept it hunting alone
-- while the others serviced (WA+PO 세작, 656s).
-- A crewmate still fighting on this floor is not left behind: the group leaves once its fight is over
-- (walking out first split the group on the floor, RO+PO and WA+PO, 610-660s).
local function servicing_on_floor(snap)
    local servicing = snap.record.memory.servicing
    if together.grouped(snap) == false or servicing == nil or servicing.leave ~= true or
        ground.floor_at(snap.world.map) == nil then
        return false
    end
    for _, member in ipairs(party.members(snap)) do
        if member.self.ghost == false and member.world.map == snap.world.map and member.task == "DEFEND" then
            return false
        end
    end
    return true
end

local function leave_for_service(snap, facts)
    local plan = party.plan(snap, facts)
    if plan == nil then
        return nil
    end
    return { kind = ACTION.MOVE, dest = { maps = { party.entry_gate(snap, facts, plan) } } }
end

return choose({
    when(servicing_on_floor, act("FIELD", leave_for_service, "leave_for_service")),
    when(function(snap, facts)
        if field.wanted(snap, facts) then
            return true
        end
        tree.postpone(snap, field.revisit(snap))
        return false
    end, choose({
        act("FIELD", occupied(rescue), "rescue"),
        together.keep,
        when(together.on_ground, on_ground),
        enter,
    })),
}, "hunt")
