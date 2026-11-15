local combat = require("brain.data.combat")
local danger = require("brain.policies.danger")
local escort = require("brain.policies.escort")
local ground = require("brain.policies.ground")
local score  = require("brain.policies.score")

local M = {}

local offers  = setmetatable({}, { __mode = "k" })
local members = setmetatable({}, { __mode = "k" })
local plans   = setmetatable({}, { __mode = "k" })

M.member = escort.member

local function summary(plan)
    if plan == nil then
        return nil
    end
    local floors = {}
    for _, floor in ipairs(plan.floors) do
        table.insert(floors, { map = floor.map, targets = floor.targets, kills = floor.kills, supply = floor.supply })
    end
    return {
        name = plan.name,
        rate = plan.rate,
        peak = plan.peak,
        pain = plan.pain,
        peak = plan.peak,
        floors = floors,
        yields = plan
            .yields,
        gate = plan.gate,
        crowd = plan.crowd
    }
end

local function arrival(snap, query, decision)
    if #(snap.group or {}) == 0 or decision == nil or decision.kind ~= ACTION.MOVE or decision.dest == nil then
        return nil
    end
    local route = query.route(decision.dest.maps or { decision.dest.map })
    return route ~= nil and snap.now + route.cost * (snap.pace.step or combat.walk) or nil
end

M.errands = { SUPPLY = true, SELL = true, OUTFIT = true, LEARN = true }

function M.stuck(snap, span)
    local spot    = snap.record.memory.spot
    local blocked = snap.record.memory.blocked
    return spot ~= nil and spot.map == snap.world.map and snap.now - spot.at >= span and type(blocked) == "table" and
        blocked.map == snap.world.map and blocked.at >= spot.at
end

-- Walled in means every tile beside me is a wall or a mob, so I cannot walk with the crew whatever I am doing
-- this moment. A poet walled in on 흉가 96 went DEFEND, REST and FIELD within one fight, and each FIELD let its
-- rogue leave for the next floor and the rally send it straight back (offline RO+PO 흉가 seed 2, 682-688s).
-- Crewmates are not walls: they step aside (together.make_way).
function M.walled(snap, query)
    local taken = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil then
            taken[mob.x .. "," .. mob.y] = true
        end
    end
    for _, side in ipairs({ { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }) do
        local x, y = snap.world.x + side[1], snap.world.y + side[2]
        if query.floor(snap.world.map, x, y) and taken[x .. "," .. y] == nil then
            return false
        end
    end
    return true
end

-- Pinned: my last walk failed and I have not moved since, though not for want of a way or for a character in it:
-- a destination no path reaches leaves me free to walk anywhere else, and a character walks on or, if a crewmate,
-- steps aside (together.make_way). Counted pinned, a rogue and a warrior that had bumped into each other held
-- their crew at the rally for good (offline profile trio seed 12, 125-300s, run 13:15). An NPC never moves, so a walk it
-- stops is a way that is not there, as a wall's: counted pinned, a mage stopped at the NPC by the gate of 평양성 held its
-- warrior at the door, and the warrior on the only tile to the door held the mage, both for minutes (online WA+MA
-- 162427, 16:28-16:36).
function M.pinned(snap)
    local blocked = snap.record.memory.blocked
    return M.stuck(snap, snap.pace.swing or combat.swing) and blocked.walls ~= true and blocked.detail ~= "CHARACTER" and
        blocked.detail ~= "NPC"
end

-- The tile of the crewmate my last walk stopped at, while I have not moved since: my walker found no way round it.
-- A walk counted on the floor alone cannot tell, as it does not keep off the mobs my walker keeps off: a poet that
-- counted a way round it two steps longer stood on its warrior's only way for 360s (offline WA+PO 세작 seed 0,
-- 540-900s, run 17:35).
function M.blocker(snap)
    local blocked = snap.record.memory.blocked
    return M.stuck(snap, 0) and blocked.detail == "CHARACTER" and blocked.by or nil
end

-- Where my walk goes, also while it stands stopped at a crewmate. Posted with the walk alone, the goal left my
-- profile between tries: a warrior stopped at the tile south of it on its way to the stairs of 흉가 92 had a goal
-- in one of two snapshots its crew took, and the crewmate on that tile makes way only for a walk with a goal
-- (offline profile trio rooted seed 6, 320-740s, run 22:38, walker held to the judged way). A walk stopped at its
-- own goal tile was not passing the crewmate on it and keeps no goal: kept, a poet whose place beside its warrior
-- the warrior had stepped onto sent the warrior off that tile at each of its tries for the stairs of 흉가 97, 26s
-- (offline profile trio seed 4, 820-846s, run 23:51, same walker). A walk to a map has no tile of its own, so the
-- tile its walker was heading for stands for it: without one, a poet standing where 흑해골굴 lets out beside its stairs
-- never made way for its warrior walking in, and the three stood at the entrance of 국내성 (online WA+PO+MA
-- 150824, 15:15:46-15:16:49).
local function going(snap, decision)
    if decision ~= nil and decision.kind == ACTION.MOVE and decision.dest ~= nil and decision.dest.x ~= nil then
        return { map = decision.dest.map, x = decision.dest.x, y = decision.dest.y }
    end
    local by      = M.blocker(snap)
    local blocked = snap.record.memory.blocked
    local dest    = by ~= nil and blocked.dest or nil
    if dest ~= nil and dest.x == nil then
        dest = blocked.goal
    end
    if dest == nil or dest.x == nil or (dest.x == by.x and dest.y == by.y) then
        return nil
    end
    return { map = dest.map, x = dest.x, y = dest.y }
end

function M.stalled(snap, span)
    local spot = snap.record.memory.spot
    return M.stuck(snap, span) or (spot ~= nil and spot.map == snap.world.map and snap.now - spot.at >= span and
        snap.self.struck ~= nil and snap.self.struck <= span)
end

-- The best mob I take here now and what it is worth, posted while I hunt so the leader can weigh staying on the floor
-- for it: the target the hunt weighed this decision, its kill and the time its fight and walk take.
local function prey(snap, query, decision)
    if decision == nil or decision.task ~= "FIELD" then
        return nil
    end
    local plan = M.plan(snap, query)
    local here = nil
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        if floor.map == snap.world.map then
            here = floor
        end
    end
    -- Prey is what I would strike (trees.hunt strike): a mage beside its warrior had a skeleton in reach whose strike
    -- was not worth its mana there, so it held back, while the skeleton kept its leader on the floor and both waited
    -- for good (online 024320, 02:47:33).
    local found = here ~= nil and require("brain.tasks.field").target(snap, query, here.targets) or nil
    if found == nil or require("brain.tasks.aid").outweighs(snap, query, found) == false then
        return nil
    end
    local fight = (found.forecast or {}).fight or {}
    return {
        map  = snap.world.map,
        oid  = found.mob.oid,
        gain = require("brain.policies.combat").gain(found.mob, snap),
        time = (fight.time or 0) + (found.steps or 0) * (snap.pace.step or combat.walk),
    }
end

-- Whether the crew stays on this floor is the leader's call, and the vanguard leads on as it says (user 11-15). The
-- crew stays while a crewmate here cannot come along (held), is busy with something else than the hunt, or has prey
-- worth more than what its fight takes at the next floor's rate: a caster shooting a skeleton across a wall keeps the
-- vanguard that cannot walk round to it from leaving it behind (healer_waits_beside_leader). Kept as a crewmate having
-- hurt a mob still alive, the crew waited five minutes on a skeleton at 1% that nobody could strike and a poison could
-- not finish (online 004028, 00:43:36-00:48:30).
local function judged(snap, query, own)
    local plan = M.plan(snap, query)
    local rate = 0
    for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
        if floor.map ~= snap.world.map then
            rate = math.max(rate, floor.rate or 0)
        end
    end
    local stay = type(own) == "table" and own.map == snap.world.map and (own.gain or 0) > (own.time or 0) / 1000 * rate
    local why  = stay and { "own:" .. tostring(own.oid) } or {}
    for _, member in ipairs(M.members(snap)) do
        if member.world.map == snap.world.map and member.self.ghost == false then
            local busy  = member.task ~= nil and member.task ~= "FIELD" and member.task ~= "NOTHING"
            local found = type(member.prey) == "table" and member.prey.map == snap.world.map and member.prey or nil
            local worth = found ~= nil and (found.gain or 0) > (found.time or 0) / 1000 * rate
            if M.held(member) then
                table.insert(why, member.self.name .. ":held")
            elseif busy then
                table.insert(why, member.self.name .. ":" .. tostring(member.task))
            elseif worth then
                table.insert(why, member.self.name .. ":prey" .. tostring(found.oid))
            end
            stay = stay or M.held(member) or busy or worth
        end
    end
    return stay, table.concat(why, ","), rate
end

local stays  = setmetatable({}, { __mode = "k" })
local stayed = {}

function M.stays(snap, query)
    if stays[snap] == nil then
        local leader = snap.leader ~= snap.self.name and M.leader(snap) or nil
        local posted = leader ~= nil and type(leader.stay) == "table" and leader.stay or nil
        if posted ~= nil and posted.map == snap.world.map then
            stays[snap] = posted.stay == true
        else
            stays[snap] = (judged(snap, query, nil))
        end
    end
    return stays[snap]
end

function M.profile(snap, query, decision)
    local grouped = #(snap.group or {}) > 0
    local base    = score.recovered(snap, query)
    local mine    = grouped and prey(snap, query, decision) or nil
    local stay    = nil
    if grouped and snap.leader == snap.self.name then
        local why, rate = nil, nil
        stay, why, rate = judged(snap, query, mine)
        local key       = tostring(snap.world.map) .. "|" .. tostring(stay)
        if stayed[snap.self.name] ~= key then
            stayed[snap.self.name] = key
            log("debug", string.format("[brain:stay] bot=%s map=%s stay=%s why=%s rate=%.3f", snap.self.name,
                tostring(snap.world.map), tostring(stay), why, rate))
        end
    end
    return {
        self      = base.self,
        spells    = snap.spells,
        gear      = base.gear,
        buffs     = snap.buffs,
        bag       = base.bag,
        pace      = snap.pace,
        world     = { map = snap.world.map, x = snap.world.x, y = snap.world.y },
        now       = snap.now,
        grouped   = grouped,
        fled      = snap.record.memory.fled,
        dry       = snap.record.memory.dry,
        servicing = snap.record.memory.servicing,
        fleeing   = grouped and snap.record.memory.fleeing or nil,
        joined    = grouped and snap.record.memory.joined or nil,
        leashed   = grouped and decision ~= nil and decision.leashed == true or nil,
        looted    = snap.record.memory.looted or 0,
        task      = decision ~= nil and decision.task or nil,
        errand    = grouped and decision ~= nil and M.errands[decision.task] and decision.dest or nil,
        -- Pinned means my last walk failed and I have not moved since, whatever I do now. Posted only while
        -- walking, a poet rooted beside its warrior healed and then waited for it to cross, looked free and was
        -- left a floor behind (offline profile pair rooted healer seed 2, 273.9s); a member drinking in a shop is
        -- not fetched for it, as fetching is for the hunting floors (hunt stage). It holds beside the crew too:
        -- posted only once apart, a rogue rooted four tiles from its warrior at the entrance looked free, and the
        -- warrior and poet went in and waited inside without it (offline profile trio rooted seed 1, 204-262s).
        pinned    = grouped and M.pinned(snap) or nil,
        blocker   = grouped and M.blocker(snap) or nil,
        walled    = grouped and snap.world.x ~= nil and M.walled(snap, query) or nil,
        peril     = grouped and M.peril(snap, query) or nil,
        dealt     = grouped and require("brain.policies.battle").threat(snap.record.memory.dealt, snap.world.map) or nil,
        fallen    = snap.record.death ~= nil and
            { map = snap.record.death.map, at = snap.now - (snap.record.death.age or 0) } or nil,
        going     = going(snap, decision),
        heading   = decision ~= nil and decision.kind == ACTION.MOVE and decision.dest ~= nil and
            (decision.dest.maps or { decision.dest.map }) or nil,
        arrives   = arrival(snap, query, decision),
        battle    = grouped and snap.record.memory.battle or nil,
        prey      = mine,
        stay      = stay ~= nil and { map = snap.world.map, stay = stay } or
            nil,
        marks     = grouped and snap.record.memory.marks or nil,
        plan      = grouped and snap.leader == snap.self.name and summary(M.plan(base, query)) or nil,
        rally     = grouped and snap.leader == snap.self.name and M.rally(snap, query) or nil,
        vanguard  = grouped and snap.leader == snap.self.name and M.vanguard_name(snap, query) or nil,
        rescue    = grouped and snap.leader == snap.self.name and M.rescue(snap, query) or nil,
    }
end

local function hazards(team, query)
    local found = {}
    for _, member in ipairs(team) do
        for _, id in ipairs(danger.perilous(member, query, false)) do
            found[id] = true
        end
    end
    return found
end

function M.perilous(snap, query)
    local members = M.members(snap)
    if #members == 0 then
        return danger.perilous(snap, query)
    end
    local found = hazards(members, query)
    for _, id in ipairs(danger.perilous(snap, query)) do
        found[id] = true
    end
    local list = {}
    for id in pairs(found) do
        table.insert(list, id)
    end
    table.sort(list)
    return list
end

function M.gate(snap, query, plan, team)
    local first  = plan.floors[1].map
    local others = {}
    for _, member in ipairs(team) do
        if member ~= snap then
            table.insert(others, member)
        end
    end
    local crew  = escort.crew(others, query)
    local route = query.route({ first }, danger.perilous(snap, query, crew), danger.shunned(snap, query))
    if route == nil then
        return nil
    end
    local maps = { snap.world.map }
    for _, hop in ipairs(route.path or {}) do
        table.insert(maps, hop.dest)
    end
    local floors = {}
    for _, floor in ipairs(plan.floors) do
        floors[floor.map] = true
    end
    local door = first
    for index = #maps, 1, -1 do
        if floors[maps[index]] then
            door = maps[index]
        end
    end
    local ids      = hazards(team, query)
    local previous = nil
    for _, map in ipairs(maps) do
        for id in pairs(ids) do
            if query.native(map, id) then
                return previous or map, door
            end
        end
        previous = map
    end
    return maps[#maps - 1] or first, door
end

function M.feasible(snap, query, plan, team)
    local gate = M.gate(snap, query, plan, team)
    if gate == nil then
        return nil
    end
    for _, member in ipairs(team) do
        local origin = { map = member.world.map, x = member.world.x, y = member.world.y }
        local route  = member == snap and
            query.route({ gate }, danger.perilous(snap, query, false), danger.shunned(snap, query)) or
            query.path({ maps = { gate } }, danger.perilous(member, query, false), {}, origin)
        if route == nil and member.world.map ~= gate then
            return nil
        end
    end
    return gate
end

local function solo(snap, query)
    return ground.choose(snap, query, ground.mode(snap))
end

-- The group's leader weighs taking one more in as two weigh a pair: everyone already in shares one rate (the server
-- splits exp evenly), so the crew gains when the larger plan's rate beats the plan it has, and the newcomer when it
-- beats hunting alone. Only pairs were weighed, and a rogue that came to its poet after the warrior hunted alone
-- beside the pair with no way in (online 123007). Only the leader adds members (group::container::handle_action).
-- Alone, each is weighed against the other hunting alone as well: two that would hunt the same ground share its
-- respawn, and taken whole, two mages each alone on 국내해골굴 outweighed hunting it together (brain unit probe).
function M.offer(snap, query)
    local grouped = #(snap.group or {}) > 0
    if snap.self.ghost or snap.self.class == "NONE" or snap.self.grouping ~= true or
        (grouped and (snap.leader ~= snap.self.name or #snap.group >= combat.party.size)) then
        return nil
    end
    if offers[snap] ~= nil then
        return offers[snap] or nil
    end
    local origin = snap
    snap         = score.recovered(snap, query)
    local team   = { snap }
    for _, member in ipairs(M.members(snap)) do
        table.insert(team, member)
    end
    local nearby = {}
    local parted = snap.record.memory.left
    local recent = {}
    if type(parted) == "table" and snap.now - (parted.at or 0) < (parted.waited or 0) * 1000 then
        for _, name in ipairs(parted.names or {}) do
            recent[name] = true
        end
    end
    local crew = snap.record.memory.crew
    for _, entry in ipairs(query.roster()) do
        local profile = entry.profile
        if type(profile) == "table" and recent[entry.name] == nil and profile.grouped == false and
            (crew == nil or crew[entry.name]) and
            profile.self ~= nil and profile.self.grouping == true and profile.self.ghost == false and
            profile.self.class ~= "NONE" then
            table.insert(nearby, entry)
        end
    end
    table.sort(nearby, function(left, right)
        local a = math.abs(left.profile.self.level - snap.self.level)
        local b = math.abs(right.profile.self.level - snap.self.level)
        return a < b or (a == b and left.name < right.name)
    end)
    if #nearby == 0 then
        offers[origin] = false
        return nil
    end
    local current = grouped and ground.party(team, query, "SUPPLY") or nil
    local alone   = grouped == false and solo(snap, query) or nil
    local best    = nil
    for rank, entry in ipairs(nearby) do
        if rank > combat.party.scan then
            break
        end
        local other   = M.member(entry.profile)
        local members = { other }
        for index = #team, 1, -1 do
            table.insert(members, 1, team[index])
        end
        local plan = ground.party(members, query, "SUPPLY")
        if plan ~= nil and M.feasible(snap, query, plan, members) == nil then
            plan = nil
        end
        if plan ~= nil then
            local theirs  = solo(other, query)
            local mine    = grouped and (current ~= nil and (current.peak or current.rate) or 0) or
                ground.contended(snap, query, alone, theirs)
            local gain    = (plan.peak or plan.rate) - mine
            local partner = (plan.peak or plan.rate) - ground.contended(other, query, theirs, alone)
            if gain > 0 and partner > 0 and (best == nil or gain > best.gain) then
                best = { name = entry.name, gain = gain, plan = plan }
            end
        end
    end
    offers[origin] = best or false
    return best
end

function M.members(snap)
    if members[snap] == nil then
        local found = {}
        for _, profile in ipairs(snap.party or {}) do
            table.insert(found, M.member(profile))
        end
        members[snap] = found
    end
    return members[snap]
end

function M.known(snap)
    return #M.members(snap) >= #(snap.group or {})
end

local held  = {}
local gates = {}

function M.forget()
    for name in pairs(held) do
        held[name] = nil
    end
end

local logged = setmetatable({}, { __mode = "v" })

local function team_of(snap, members)
    local team = { snap }
    for _, member in ipairs(members) do
        table.insert(team, member)
    end
    return team
end

local function committed(snap, members)
    return #members == 0 and snap.record.memory.solo or snap.record.memory.commit
end

function M.key(snap, query)
    local members = M.members(snap)
    local parts   = { (snap.now or 0) // combat.rerank, tostring(committed(snap, members)),
        table.concat(danger.shunned(snap, query), ",") }
    for _, member in ipairs(team_of(snap, members)) do
        table.insert(parts, score.signature(member, ""))
        table.insert(parts, tostring(member.self.ghost))
        table.insert(parts, tostring(ground.emptied(member)))
    end
    return table.concat(parts, "|")
end

function M.compute(snap, query)
    local members = M.members(snap)
    local team    = team_of(snap, members)
    local commit  = committed(snap, members)
    local plan    = commit ~= nil and ground.hold(team, query, commit) or nil
    return plan or ground.party(team, query, "SUPPLY")
end

function M.plan(snap, query)
    local members = M.members(snap)
    if #members > 0 and snap.leader ~= nil and snap.leader ~= snap.self.name then
        for _, member in ipairs(members) do
            if member.self.name == snap.leader and member.plan ~= nil then
                return member.plan
            end
        end
    end
    if plans[snap] == nil then
        local team   = team_of(snap, members)
        local commit = committed(snap, members)
        local key    = M.key(snap, query)
        local kept   = held[snap.self.name]
        local made   = require("brain.planner").current(snap, query)
        local plan   = nil
        if made ~= nil then
            plan = made.plan
        elseif kept ~= nil and kept.key == key then
            plan = kept.plan
        else
            plan = M.compute(snap, query)
            held[snap.self.name] = { key = key, plan = plan }
        end
        if plan ~= nil and plan ~= logged[snap.self.name] and plan.floors[1] ~= nil and query.demand ~= nil then
            logged[snap.self.name] = plan
            local names = {}
            for _, member in ipairs(team) do
                table.insert(names, member.self.name)
            end
            log("debug", string.format("[brain:plan] bot=%s ground=%s first=%d others=%.3f committed=%s",
                snap.self.name, tostring(plan.name), plan.floors[1].map, query.demand(plan.floors[1].map, names),
                tostring(commit ~= nil)))
        end
        if plan ~= nil then
            local copy = {}
            for key, entry in pairs(plan) do
                copy[key] = entry
            end
            -- The entrance is where the group came in from, so it is taken while outside and kept inside: worked
            -- out from a floor, the way to the first floor is empty and the entrance came out as the floor itself,
            -- and a poet leader leaving 91 for supplies walked to where it stood for 60s while its rogue fought
            -- alone a floor deeper (offline RO+PO seed 2, 563-622s).
            local inside = false
            for _, floor in ipairs(plan.floors) do
                inside = inside or floor.map == snap.world.map
            end
            -- Worked out from behind the ground, the way to the first floor runs through the other floors and the
            -- entrance comes out as one of them: the entrance the group came in from stays then. Taken anew, a
            -- warrior back from the shop past the last floor of 연청산소 walked to an entrance on its second floor
            -- and waited five floors in, while its poet waited at the door they had come in by and then walked
            -- those floors alone, 72s in every one of nine seeds (offline WA40+PO40 seeds 0-8, run 06:55).
            local kept = gates[snap.self.name]
            if inside and kept ~= nil and kept.name == plan.name then
                copy.gate = kept.map
            else
                local gate, first = M.gate(snap, query, plan, team)
                local floored     = false
                for _, floor in ipairs(plan.floors) do
                    floored = floored or floor.map == gate
                end
                if floored and kept ~= nil and kept.name == plan.name then
                    copy.gate = kept.map
                else
                    copy.gate = gate or plan.floors[1].map
                    if inside == false and floored == false then
                        gates[snap.self.name] = { name = plan.name, map = copy.gate, first = first or plan.floors[1].map }
                    end
                end
            end
            plan = copy
        end
        plans[snap] = plan or false
    end
    return plans[snap] or nil
end

-- What the crew on a member's floor goes without while that member is away: each crewmate left there loses that time
-- at the worth the caller weighs with. Valued at the group plan's rate instead, the plan was worked out from fights
-- priced with this very cost: a warrior's plan without potions rose from 19,742 to 28,520 and it skipped its first
-- supply (online 105006, 10:52).
function M.alone(snap, member, seconds, worth)
    local crew = { snap }
    for _, other in ipairs(M.members(snap)) do
        table.insert(crew, other)
    end
    local left = 0
    for _, other in ipairs(crew) do
        if other.self.name ~= member.self.name and other.world.map == member.world.map and other.self.ghost == false then
            left = left + 1
        end
    end
    return left * seconds * worth.value
end

-- A death costs what the dead member loses and what its crew on that floor goes without until it is back, whoever
-- dies. Priced with the crew for my own death only, a poet healed itself before its warrior, and the warrior, short
-- of the heals that kept it up, fled doomed 7 times against 3 (offline 15-minute 9 seeds, run 090652).
function M.death(snap, query, member, worth)
    return danger.loss(member, query, worth) + M.alone(snap, member, danger.revival(member, query), worth)
end

function M.leader(snap)
    for _, member in ipairs(M.members(snap)) do
        if member.self.name == snap.leader then
            return member
        end
    end
    return nil
end

-- The leader commands (ground, floors, servicing) and the vanguard walks in front of the crew on the floor.
-- The vanguard is the crewmate that fights beside its prey and stands the most blows, else the one that
-- stands the most blows: a poet leader walked on to the next floor while its warrior fought, and the two
-- met again only 80s later (online group, 16:54). Fighting beside its prey means its best single blow lands
-- next to the mob; compared by sustained damage, a poet whose mana did not refill counted as a melee hitter. The leader names it once per crew and members read the
-- name, so a stale profile never makes two members walk in front, and the front does not swap mid-hunt.
local vanguards = {}

local function ahead(left, right)
    if left.melee ~= right.melee then
        return left.melee
    elseif left.hp ~= right.hp then
        return left.hp > right.hp
    elseif left.level ~= right.level then
        return left.level > right.level
    end
    return left.name < right.name
end

function M.vanguard_name(snap, query)
    if #(snap.group or {}) == 0 then
        return nil
    end
    local lead = snap.leader ~= snap.self.name and M.leader(snap) or nil
    if lead ~= nil and lead.vanguard ~= nil then
        return lead.vanguard
    end
    local team  = { snap }
    local names = { snap.self.name }
    for _, member in ipairs(M.members(snap)) do
        table.insert(team, member)
        table.insert(names, member.self.name)
    end
    table.sort(names)
    local key  = table.concat(names, "|")
    local kept = snap.leader == snap.self.name and vanguards[snap.self.name] or nil
    if kept == nil or kept.key ~= key then
        local best = nil
        for _, member in ipairs(team) do
            local entry = {
                name  = member.self.name,
                melee = score.melee(member, query),
                hp    = member.self.max_hp or 0,
                level = member.self.level or 0,
            }
            if best == nil or ahead(entry, best) then
                best = entry
            end
        end
        kept = { key = key, name = best.name }
        if snap.leader == snap.self.name then
            vanguards[snap.self.name] = kept
        end
    end
    return kept.name
end

function M.vanguard(snap, query)
    local name = M.vanguard_name(snap, query)
    for _, member in ipairs(M.members(snap)) do
        if member.self.name == name then
            return member
        end
    end
    return nil
end

-- What the mobs beside me take and what I set against them, posted so the leader can weigh a rescue for a
-- crewmate it cannot see. What I set against them is the damage I have dealt them over the time I have been at it:
-- a poet healing under blows kills nothing. Counted by the spell damage it could deal while it fought one, a poet
-- swinging at a skeleton at about 110 a second looked able to end it at 1070, so no draw was named and it swung on
-- (offline profile 63+65, run 02:05), and a rogue's 필살검무, no sustained spell, counted for nothing. A mob beside
-- me that comes for a crewmate (chaser) is not on me: counted as mine, 11 of the 56 spells strikers cast at mobs on
-- no crewmate went at a mob whose most damage the striker had dealt itself (offline 25 teams, run 15:23).
function M.peril(snap, query)
    local battle    = require("brain.policies.battle")
    local fight     = require("brain.policies.combat")
    local attackers = {}
    local drains    = {}
    local drain     = 0
    local total     = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        local chaser = mob.x ~= nil and M.chaser(snap, mob, snap.world) or nil
        if mob.x ~= nil and (mob.speed or 0) > 0 and battle.attacking(snap, mob) and
            (chaser == nil or chaser.name == snap.self.name) then
            local _, high             = fight.spread(snap, mob)
            drains[tostring(mob.oid)] = high * 1000 / mob.speed
            drain                     = drain + drains[tostring(mob.oid)]
            total                     = total + (mob.hp or 0) * (mob.ratio or 1)
            table.insert(attackers, mob.oid)
        end
    end
    if #attackers == 0 then
        return nil
    end
    local dealt  = battle.threat(snap.record.memory.dealt, snap.world.map)
    local damage = 0
    local since  = snap.now
    for _, oid in ipairs(attackers) do
        local entry = dealt[tostring(oid)]
        if entry ~= nil then
            damage = damage + entry.damage
            since  = math.min(since, entry.since or entry.at)
        end
    end
    local dps = damage > 0 and damage * 1000 / math.max(snap.now - since, fight.cast(snap)) or 0
    return { drain = drain, drains = drains, hp = total, dps = dps, attackers = attackers }
end

-- The leader names one crewmate to save and the one who goes. Saving is worth the death chance it removes
-- times what that death costs, the chance counted as in a heal's rescue from the blows left after the heals
-- crewmates in sight give it. The blows stop once the helper has drawn the attackers off: a mob turns on whoever
-- hurt it most, so the helper outdoes the crewmate's damage on each, with a spell from where it stands or with
-- blows once it has walked there (user, 11-12). Counted until the attackers died instead, a poet three skeletons
-- struck was not worth a warrior's walk of seven tiles (brain unit test, rescuer draws). The attackers then
-- strike the helper until the crew kills them, a chance of its own death counted the same way against the heals
-- the others give it, so a poet is not sent to take a rogue's mobs on. Its own heals do not count, as healing
-- itself is what a poet under blows must stop doing to keep its crew alive. Going costs the hunting the helper
-- gives up meanwhile, priced by the crew's one plan. A rogue that kills its attackers itself is worth little and a
-- poet that cannot is worth its life: a poet under blows waited a minute for a warrior held by its own attackers
-- (offline stage supply seed 2, 165-210s).
-- A helper's spells count only where they pay for their mana as the helper prices its own strikes (aid.outweighs):
-- counted in full, the poet beside a 건곤대나이 warrior was the one sent and then kept its mana for heals, so the
-- warrior was never sent (offline profile trio rooted seed 2, 516.8s, run 21:55).
local rescues = setmetatable({}, { __mode = "k" })

local function casts_pay(snap, helper, team, attacker, strong)
    if attacker == nil or strong.blow.mana <= 0 then
        return true
    end
    local crew = {}
    for _, other in ipairs(team) do
        if other ~= helper then
            table.insert(crew, other.member)
        end
    end
    local value = require("brain.policies.value")
    local share = strong.blow.ranged / math.max(attacker.hp or 1, 1) *
        require("brain.policies.combat").gain(attacker, helper.member)
    return share >= strong.blow.mana * value.fuel(snap, helper.member, crew, attacker)
end

local function dealt_by(snap, member)
    if member == snap then
        return require("brain.policies.battle").threat(snap.record.memory.dealt, snap.world.map) or {}
    end
    return member.dealt or {}
end

local function sighted(from, to)
    return math.abs(from.x - to.x) <= combat.sight.x and math.abs(from.y - to.y) <= combat.sight.y
end

-- The crewmate a mob comes for, as the server picks its target (ai::find_target_in_sight): the one that dealt it
-- most within its memory of blows, else the nearest in its sight. I am counted where I would stand. A heal makes
-- no aggro, so a poet that only healed is chased only for being nearest.
function M.chaser(snap, mob, at)
    local crew = { { name = snap.self.name, world = at, dealt = dealt_by(snap, snap) } }
    for _, member in ipairs(M.members(snap)) do
        if member.self.ghost == false and member.world.map == snap.world.map and member.world.x ~= nil then
            table.insert(crew, { name = member.self.name, world = member.world, dealt = dealt_by(snap, member) })
        end
    end
    local best, most = nil, 0
    for _, entry in ipairs(crew) do
        local hit = (entry.dealt[tostring(mob.oid)] or {}).damage or 0
        if hit > most then
            best, most = entry, hit
        end
    end
    local near = math.huge
    for _, entry in ipairs(best == nil and crew or {}) do
        local gap = math.abs(entry.world.x - mob.x) + math.abs(entry.world.y - mob.y)
        if sighted(mob, entry.world) and gap < near then
            best, near = entry, gap
        end
    end
    return best
end

-- The time a helper takes to draw the attackers, and whether it walks for any of them. A spell draws from where the
-- helper stands only while its mana lasts: counted for a rogue at 0 mp that 필살검무 had emptied, the draw named no
-- walk, and the mobs in its way to its poet were worth nothing to it (offline RO+PO 세작, run 23:21). A helper whose
-- strike spends all its mana keeps none and drinks what each spell needs (recovery.spare), one drink before the cast.
local function drawing_time(snap, helper, team, peril, strong, spells, arrive)
    local fight    = require("brain.policies.combat")
    local recovery = require("brain.policies.recovery")
    local cast     = fight.cast(helper.member) / 1000
    local swing    = ((helper.member.pace or {}).swing or combat.swing) / 1000
    local mana     = helper.member.self.mp or 0
    local top      = helper.member.self.max_mp or 0
    local doses    = recovery.draining(helper.member) and recovery.doses(helper.member, "mp") or {}
    local seen     = {}
    for _, mob in ipairs(snap.world.mobs or {}) do
        seen[mob.oid] = mob
    end
    local ranged, close = 0, 0
    for _, oid in ipairs(peril.attackers) do
        local lead, mine = 0, 0
        for _, other in ipairs(team) do
            local entry  = dealt_by(snap, other.member)[tostring(oid)]
            local damage = entry ~= nil and entry.damage or 0
            if other == helper then
                mine = damage
            else
                lead = math.max(lead, damage)
            end
        end
        local need  = math.max(0, lead - mine)
        local mob   = seen[oid]
        local casts = strong.blow.ranged > 0 and math.floor(need / strong.blow.ranged) + 1 or 0
        local cost  = casts * strong.blow.mana
        local dose  = nil
        for _, entry in ipairs(cost > mana and doses or {}) do
            dose = dose or (mana + math.min(entry.amount, top - mana) >= cost and entry or nil)
        end
        if spells and casts > 0 and (cost <= mana or dose ~= nil) and mob ~= nil and mob.x ~= nil and
            sighted(helper.member.world, mob) then
            ranged = ranged + casts * cast + (dose ~= nil and cast or 0)
            mana   = mana + (dose ~= nil and math.min(dose.amount, top - mana) or 0) - cost
        else
            close = close + (math.floor(need / math.max(strong.blow.adjacent, 1e-9)) + 1) * swing
        end
    end
    return ranged + (close > 0 and arrive + close or 0), close > 0
end

local function exposure(snap, query, helper, team, peril, worth, span)
    local fight = require("brain.policies.combat")
    local hits  = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        for _, oid in ipairs(peril.attackers) do
            if mob.oid == oid and (mob.speed or 0) > 0 then
                local _, high = fight.spread(helper.member, mob)
                hits          = hits + high * 1000 / mob.speed
            end
        end
    end
    local mend = 0
    for _, other in ipairs(team) do
        local world = other.member.world
        if other ~= helper and world.map == helper.member.world.map and world.x ~= nil and
            sighted(world, helper.member.world) then
            mend = mend + score.capability(other.member, query).hps.others
        end
    end
    local bleed = hits - mend
    if bleed <= 0 or span <= 0 then
        return 0
    end
    return math.exp(-helper.member.self.hp / (bleed * span)) * M.death(snap, query, helper.member, worth)
end

-- The blows a crewmate takes until its attackers are drawn off cost it the hp they take, priced as it gets that hp
-- back, which for a poet is the mana its crew's strikes burn as fuel: priced by the chance of its death alone, a
-- poet struck at full hp was not worth a rescue, and the warrior beside it fought its own four mobs while three
-- more beat the poet (offline 25 teams, run 20:53; user 11-12).
local function struck_price(snap, query, victim, team, worth, attacker)
    local value = require("brain.policies.value")
    local crew  = {}
    for _, other in ipairs(team) do
        if other ~= victim then
            table.insert(crew, other.member)
        end
    end
    local mana  = math.max(value.refill(victim.member, query, worth),
        value.fuel(snap, victim.member, crew, attacker))
    local price = value.vigor(victim.member, query, worth, mana)
    return price < math.huge and price or 0
end

local function weigh_rescue(snap, query)
    local worth = require("brain.policies.value").baseline(snap, query)
    local team  = { { member = snap, peril = M.peril(snap, query) } }
    for _, member in ipairs(M.members(snap)) do
        if member.self.ghost == false then
            table.insert(team, { member = member, peril = member.peril })
        end
    end
    local best = nil
    for _, victim in ipairs(team) do
        local peril  = victim.peril
        local member = victim.member
        if peril ~= nil and member.world.x ~= nil and ground.floor_at(member.world.map) ~= nil then
            local healing = 0
            for _, other in ipairs(team) do
                local world = other.member.world
                if other ~= victim and world.map == member.world.map and world.x ~= nil and
                    math.abs(world.x - member.world.x) <= combat.sight.x and
                    math.abs(world.y - member.world.y) <= combat.sight.y then
                    healing = healing + score.capability(other.member, query).hps.others
                end
            end
            local drain = peril.drain - healing
            if drain > 0 and member.self.hp > 0 then
                local alone    = peril.dps > 0 and peril.hp / peril.dps or math.huge
                local risk     = math.exp(-member.self.hp / (drain * alone))
                local loss     = M.death(snap, query, member, worth)
                local attacker = nil
                for _, mob in ipairs(snap.world.mobs or {}) do
                    for _, oid in ipairs(peril.attackers or {}) do
                        attacker = attacker or (mob.oid == oid and mob or nil)
                    end
                end
                local price = struck_price(snap, query, victim, team, worth, attacker)
                for _, helper in ipairs(team) do
                    local world  = helper.member.world
                    local strong = helper ~= victim and score.capability(helper.member, query) or nil
                    local spells = strong ~= nil and casts_pay(snap, helper, team, attacker, strong)
                    local dps    = strong ~= nil and (spells and strong.dps.single or strong.dps.melee) or 0
                    local burst  = strong ~= nil and (spells and strong.burst or strong.blow.strike) or 0
                    if dps > 0 and world.map == member.world.map and world.x ~= nil then
                        local steps = escort.steps(snap, query, helper.member,
                            { { x = member.world.x, y = member.world.y } })[1]
                        if type(steps) == "number" then
                            local arrive       = steps * ((helper.member.pace or {}).step or combat.walk) / 1000
                            local drawn, walks = drawing_time(snap, helper, team, peril, strong, spells, arrive)
                            local left         = math.max(0, peril.hp - peril.dps * drawn - burst)
                            local ended        = drawn + left / (peril.dps + dps)
                            local saved        = (risk - math.exp(-member.self.hp / (drain * drawn))) * loss +
                                drain * math.max(0, ended - drawn) * price
                            local taken        = exposure(snap, query, helper, team, peril, worth, ended - drawn)
                            local net          = saved - taken - drawn * worth.value
                            if net > 0 and (best == nil or net > best.value) then
                                local shares = {}
                                for oid, each in pairs(peril.drains or {}) do
                                    shares[oid] = each / peril.drain
                                end
                                best = {
                                    name = member.self.name,
                                    by = helper.member.self.name,
                                    value = net,
                                    walks = walks,
                                    shares = shares,
                                }
                            end
                        end
                    end
                end
            end
        end
    end
    return best
end

function M.rescue(snap, query)
    if rescues[snap] == nil then
        rescues[snap] = weigh_rescue(snap, query) or false
    end
    return rescues[snap] or nil
end

-- The crewmate the leader sent me to save, with the rescue it named.
function M.rescued(snap, query)
    local rescue = snap.leader == snap.self.name and M.rescue(snap, query) or (M.leader(snap) or {}).rescue
    if rescue == nil or rescue.by ~= snap.self.name then
        return nil
    end
    for _, member in ipairs(M.members(snap)) do
        if member.self.name == rescue.name and member.self.ghost == false then
            return member, rescue
        end
    end
    return nil
end

-- The rescue the leader named for me, with the crewmate it sends.
function M.awaited(snap, query)
    local rescue = snap.leader == snap.self.name and M.rescue(snap, query) or (M.leader(snap) or {}).rescue
    if rescue == nil or rescue.name ~= snap.self.name or rescue.by == snap.self.name then
        return nil
    end
    for _, member in ipairs(M.members(snap)) do
        if member.self.name == rescue.by and member.self.ghost == false then
            return rescue, member
        end
    end
    return nil
end

-- A rescuer's share in a mob: the rescue's worth in the share of the crewmate's blows that mob deals, and the whole
-- rescue for one beside me that stands nearer that crewmate than I do, so the way is cut open before the crewmate's
-- attackers. Counted whole for every mob beside the crewmate, the rescue went to mobs that struck nobody, and with the
-- rescue worth the blows its draw ends a warrior cast 백열장 at them 11,398 times in a run (offline, run 18:35).
-- The way matters only when the rescue walks: with spells that draw from where it stands, a warrior its own mobs
-- surrounded fought them for the whole rescue (1.49M) over the 백열장 at the skeleton on its poet, worth 89% of it
-- (offline profile 63+65, run 23:21; user 11-12).
function M.rescue_share(snap, query, mob)
    local victim, rescue = M.rescued(snap, query)
    if victim == nil or victim.world.map ~= snap.world.map or victim.world.x == nil or mob.x == nil then
        return 0
    end
    local share = (rescue.shares or {})[tostring(mob.oid)]
    if share ~= nil then
        return rescue.value * share
    end
    if rescue.walks and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
        local steps = escort.steps(snap, query, victim,
            { { x = mob.x, y = mob.y }, { x = snap.world.x, y = snap.world.y } })
        if type(steps[1]) == "number" and type(steps[2]) == "number" and steps[1] < steps[2] then
            return rescue.value
        end
    end
    return 0
end

-- Rally point: where a scattered group meets. Only the leader decides it and publishes it in its
-- profile; members read it instead of each picking a member to walk to. Before this, the leader and
-- members each chose where to meet with their own first-match branches (left_behind, lead_gate,
-- step_with_crew, follow_entry), so they crossed, both waited for each other, or bounced between two
-- members. One decision in one place removes those loops.
--
-- Every member's position is a candidate. A candidate costs, for each other member, the time it needs
-- to walk there (valued at that member's own exp per second) plus the expected loss of walking alone
-- across hunting floors on the way (the floor's death rate for that member alone, times the time spent
-- crossing it, times what a death costs that member). A member that cannot walk now (moves keep failing,
-- or it is fighting) cannot come, so candidates that need it to move are worse than any that do not;
-- the first comparison key is how many such members a candidate would need, the second its cost.
-- Walking toward the chosen point lowers its cost, so the choice does not flip back while members move.
local rallies = setmetatable({}, { __mode = "k" })

function M.held(member)
    return member.pinned == true or member.task == "DEFEND" or member.walled == true
end

local function walk_cost(member, query, target, worth, loss, companions)
    if member.world.map == target.map and member.world.x ~= nil and
        math.abs(member.world.x - target.x) + math.abs(member.world.y - target.y) <= combat.adjacent then
        return 0
    end
    if member.world.x == nil then
        return math.huge
    end
    local step  = (member.pace or {}).step or combat.walk
    local tiles = 0
    local path  = {}
    if member.world.map == target.map then
        tiles = math.abs(member.world.x - target.x) + math.abs(member.world.y - target.y)
    else
        local route = query.path({ maps = { target.map } }, {}, {},
            { map = member.world.map, x = member.world.x, y = member.world.y, level = member.self.level })
        if route == nil then
            return math.huge
        end
        path  = route.path or {}
        tiles = route.cost + (route.x ~= nil and math.abs(route.x - target.x) + math.abs(route.y - target.y) or 0)
    end
    local seconds  = tiles * step / 1000
    local crossing = #path > 0 and seconds / #path or 0
    local rested   = score.rested(member, ground.mode(member))
    -- Members leaving from the same spot walk the way together, so the floors are judged with them as
    -- crew; judging each alone made a leader and its escort look as exposed as a lone healer.
    local crew     = #companions > 0 and ground.crew(companions, query) or nil
    local risk     = 0
    for index = 1, #path - 1 do
        local floor = ground.floor_at(path[index].dest)
        if floor ~= nil then
            local exposed = ground.evaluate(rested, query, floor, score.capability(rested, query), crew)
            risk = risk + (exposed.deaths or 0) * crossing * loss
        end
    end
    return seconds * worth + risk
end

local function compute_rally(snap, query)
    local value = require("brain.policies.value")
    local team  = { snap }
    for _, member in ipairs(M.members(snap)) do
        if member.self.ghost == false then
            table.insert(team, member)
        end
    end
    -- The vanguard sets the group's direction on the floor and does not walk back: while it stands on a
    -- hunting floor (of the plan or of the ground it is leaving) the group meets where it is. Comparing walk
    -- costs alone sent the front back to a member when the two costs were close, although both had to go on
    -- to its floor anyway. Only a member that cannot come (moves keep failing, or it is fighting) is fetched
    -- by the cost comparison below, and that includes the leader itself while its walks keep failing now: a
    -- poet leader kept off by a skeleton on its way to the warrior waited 80s for a warrior that was walking
    -- to it (online group, 16:54). Only a threat holds it so: a walk that failed long ago and was not tried
    -- again does not (leader_ignores_rooted_member: 50s old, the rooted member had to be fetched), nor a
    -- crewmate in its way, which put the rally on the leader and left a rooted rogue a floor behind (offline
    -- trio rooted seed 13, 202-260s).
    local swing = snap.pace.swing or combat.swing
    local front = team[1]
    local named = M.vanguard_name(snap, query)
    for _, member in ipairs(team) do
        if member.self.name == named then
            front = member
        end
    end
    -- Blows alone do not hold a member where it stands: it can still walk with its attackers. Counting them,
    -- a warrior and a poet hit at once tied as candidates, the rally fell on the warrior, and it stood under its
    -- own attackers while the poet could not reach it (offline stage supply seed 2, 165-210s). A crewmate under
    -- blows that cannot win is fetched by the rescue the leader names instead (rescue).
    local function fixed(member)
        if member == snap then
            local blocked = snap.record.memory.blocked
            return M.stuck(snap, swing) and blocked.detail == "THREAT" and snap.now - blocked.at < combat.rerank
        end
        return M.held(member)
    end
    local held = false
    for _, member in ipairs(team) do
        held = held or (member ~= front and fixed(member))
    end
    if held == false and front.world.x ~= nil and ground.floor_at(front.world.map) ~= nil then
        return { name = front.self.name, map = front.world.map, x = front.world.x, y = front.world.y, blocked = 0, cost = 0 }
    end
    -- A leader outside the ground goes in to the crew already hunting there; the crew does not walk
    -- out to the leader (it left the floor for the leader's shop and the group spent the run walking).
    -- A member that cannot come stays a candidate wherever it stands: left out for standing outside, a
    -- rogue rooted at the entrance was waited for inside by its warrior and poet for good (offline profile
    -- trio rooted seed 1, 207-262s).
    local inside = {}
    for _, member in ipairs(team) do
        if member.world.x ~= nil and (ground.floor_at(member.world.map) ~= nil or fixed(member)) then
            table.insert(inside, member)
        end
    end
    local candidates = #inside > 0 and inside or team
    local prices = {}
    for index, member in ipairs(team) do
        local worth = value.baseline(member, query)
        prices[index] = { worth = worth.value, loss = M.death(snap, query, member, worth) }
    end
    local best = nil
    for _, candidate in ipairs(candidates) do
        if candidate.world.x ~= nil then
            local target  = { map = candidate.world.map, x = candidate.world.x, y = candidate.world.y }
            local blocked = 0
            local cost    = 0
            for index, member in ipairs(team) do
                if member ~= candidate then
                    local companions = {}
                    for _, other in ipairs(team) do
                        if other ~= member and other ~= candidate and fixed(other) == false and
                            other.world.map == member.world.map and other.world.x ~= nil and member.world.x ~= nil and
                            math.abs(other.world.x - member.world.x) + math.abs(other.world.y - member.world.y) <=
                            combat.adjacent then
                            table.insert(companions, score.rested(other, ground.mode(other)))
                        end
                    end
                    -- A member that cannot come reaches no spot but its own, however near: counted there when four
                    -- tiles off in a straight line, a poet walled in by scorpions beside a wall kept the rally on its
                    -- rogue, which waited while the poet walked into the scorpions for nine minutes (offline RO+PO 흉가
                    -- seed 1, 330-870s, run 07:36).
                    local walk = walk_cost(member, query, target, prices[index].worth, prices[index].loss, companions)
                    if fixed(member) or walk == math.huge then
                        blocked = blocked + 1
                    elseif walk < math.huge then
                        cost = cost + walk
                    end
                end
            end
            if best == nil or blocked < best.blocked or (blocked == best.blocked and cost < best.cost) then
                best = {
                    name = candidate.self.name,
                    map = target.map,
                    x = target.x,
                    y = target.y,
                    blocked = blocked,
                    cost = cost
                }
            end
        end
    end
    return best
end

-- Coming onto a floor of the plan that is not the one its entrance opens on, while a crewmate is still outside the
-- ground, is passing through on the way to gather, not coming in: the group comes in together from the entrance
-- (user 11-09). With the crew already inside there is nobody to gather with and any floor is coming back to it.
-- Known only while the entrance is, so a bot that never stood outside this ground takes any entry as coming in.
function M.passing(snap, query, map)
    local plan = map ~= nil and M.plan(snap, query) or nil
    local kept = gates[snap.self.name]
    if plan == nil or kept == nil or kept.name ~= plan.name or map == kept.first then
        return false
    end
    local floor = false
    for _, entry in ipairs(plan.floors) do
        floor = floor or entry.map == map
    end
    local outside = false
    for _, member in ipairs(M.members(snap)) do
        outside = outside or (member.self.ghost == false and ground.same_ground(member.world.map, map) == false)
    end
    return floor and outside
end

-- The floor I came into the ground by and when: the one noted at my entry, and on a hunting floor whose entry is not
-- noted yet the one I stand on now, as the entry is noted only after the decision (decide.entered).
local function doorstep(snap)
    local entry = snap.record.memory.entry
    if entry ~= nil then
        return entry.map, entry.at
    elseif ground.floor_at(snap.world.map) ~= nil then
        return snap.world.map, snap.now
    else
        return nil, nil
    end
end

function M.passing_through(snap, query)
    return M.passing(snap, query, (doorstep(snap)))
end

-- An entry is coming into the ground when it is onto the ground I hunt and not passing through it. Counted for any
-- hunting map, the two maps a warrior crossed on its way back from the shop behind 연청산소 made it back in the
-- ground before it reached it, its servicing ended there, and it stood on the floor it came out on (offline
-- WA40+PO40 seed 0, 687-706s, run 07:06).
function M.entering(snap, query, map)
    local plan = M.plan(snap, query)
    return (plan == nil or ground.same_ground(map, plan.floors[1].map)) and M.passing(snap, query, map) == false
end

-- Back in the ground since the servicing began: I last entered it after the servicing began, and on a hunting floor
-- whose entry is not noted yet I am entering it now, as the entry is noted only after the decision (decide.entered).
-- Counting only the entry noted before, a poet that followed its warrior back in after servicing cast a curse
-- first, then took its servicing as unfinished and walked out to town and back (offline WA+PO seeds 0-2, 375-392s).
-- A floor passed through on the way to the entrance is no coming back (passing).
function M.returned(snap, query)
    local memory   = snap.record.memory
    local door, at = doorstep(snap)
    at             = at or memory.joined
    return memory.servicing ~= nil and at ~= nil and at > memory.servicing.at and M.passing(snap, query, door) == false
end

-- After any member flees, the whole group leaves and meets in front of the entrance. Until this bot has
-- entered a ground again after the latest flight in the group (joined is its last entry time), a leader
-- still inside is about to leave too, so a member waits at the entrance instead of walking back in alone.
-- Comparing times instead of clearing a flag means a member that decides late still knows.
function M.regrouping(snap)
    local latest = nil
    local own    = snap.record.memory.fleeing
    if own ~= nil and own.at ~= nil then
        latest = own.at
    end
    for _, member in ipairs(M.members(snap)) do
        local notice = member.fleeing
        if notice ~= nil and notice.at ~= nil and (latest == nil or notice.at > latest) then
            latest = notice.at
        end
    end
    -- A leader that has entered again since the flight has led the group back in; the regroup is over
    -- and the member follows it in. Without this a member that missed the crossing waited at the gate
    -- while the leader waited inside (WA9+PO, 3178/5191).
    local joined = snap.record.memory.joined
    local lead   = snap.leader ~= snap.self.name and M.leader(snap) or nil
    local led    = lead ~= nil and lead.joined ~= nil and latest ~= nil and lead.joined > latest
    return latest ~= nil and (joined == nil or latest > joined) and led == false
end

-- A leader on a hunting floor is not always leading the hunt there. One that holds a servicing begun after it last
-- came in is about to leave as its crew did, and one walking to the entrance is on its way to gather there. Taken
-- as leading in, a warrior four seconds from walking out to the shop drew its poet, back at the entrance from its own
-- servicing, onto the first floor, where the poet stood alone for 74s and fled as the warrior reached the entrance
-- (offline WA40+PO40 seed 8, 661-736s, run 06:28).
function M.outbound(snap, gate)
    local lead = snap.leader ~= snap.self.name and M.leader(snap) or nil
    if lead == nil then
        return false
    end
    local servicing = lead.servicing
    local gathering = servicing ~= nil and servicing.leave == true and
        (lead.joined == nil or lead.joined <= servicing.at)
    for _, map in ipairs(lead.heading or {}) do
        gathering = gathering or map == gate
    end
    return gathering
end

-- The entrance the group meets at before going in. The gate the leader published is computed from where
-- the leader stands; a leader still inside would publish a floor, so a member then takes the gate from
-- its own spot.
function M.entry_gate(snap, query, plan)
    for _, floor in ipairs(plan.floors) do
        if plan.gate == floor.map then
            local team = { snap }
            for _, member in ipairs(M.members(snap)) do
                table.insert(team, member)
            end
            return M.gate(snap, query, plan, team) or plan.floors[1].map
        end
    end
    return plan.gate or plan.floors[1].map
end

-- A group needs a rally point when it is scattered, and also while a member is held where it stands
-- (fighting, or its moves keep failing): the group gathers on it instead of walking off, even if it is
-- near enough to count as together. Without this a leader left the map while a crewmate fought.
function M.rally(snap, query)
    local held = false
    for _, member in ipairs(M.members(snap)) do
        held = held or (member.self.ghost == false and M.held(member))
    end
    if #M.members(snap) == 0 or (escort.complete(snap, query) and held == false) then
        return nil
    end
    if snap.leader ~= snap.self.name then
        local lead = M.leader(snap)
        return lead ~= nil and lead.rally or nil
    end
    if rallies[snap] == nil then
        rallies[snap] = compute_rally(snap, query) or false
    end
    return rallies[snap] or nil
end

return M
