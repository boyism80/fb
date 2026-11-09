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
        table.insert(floors, { map = floor.map, targets = floor.targets })
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
        gate = plan.gate
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

function M.stalled(snap, span)
    local spot = snap.record.memory.spot
    return M.stuck(snap, span) or (spot ~= nil and spot.map == snap.world.map and snap.now - spot.at >= span and
        snap.self.struck ~= nil and snap.self.struck <= span)
end

function M.profile(snap, query, decision)
    local grouped = #(snap.group or {}) > 0
    local base    = score.recovered(snap, query)
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
        pinned    = grouped and escort.complete(snap, query) == false and
            M.stuck(snap, snap.pace.swing or combat.swing) or
            nil,
        going     = decision ~= nil and decision.kind == ACTION.MOVE and decision.dest ~= nil and decision.dest.x ~= nil and
            { map = decision.dest.map, x = decision.dest.x, y = decision.dest.y } or nil,
        heading   = decision ~= nil and decision.kind == ACTION.MOVE and decision.dest ~= nil and
            (decision.dest.maps or { decision.dest.map }) or nil,
        arrives   = arrival(snap, query, decision),
        battle    = grouped and snap.record.memory.battle or nil,
        marks     = grouped and snap.record.memory.marks or nil,
        plan      = grouped and snap.leader == snap.self.name and summary(M.plan(base, query)) or nil,
        rally     = grouped and snap.leader == snap.self.name and M.rally(snap, query) or nil,
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
    local ids      = hazards(team, query)
    local previous = nil
    for _, map in ipairs(maps) do
        for id in pairs(ids) do
            if query.native(map, id) then
                return previous or map
            end
        end
        previous = map
    end
    return maps[#maps - 1] or first
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
    local best = ground.choose(snap, query, ground.mode(snap))
    return best ~= nil and (best.peak or best.rate) or 0
end

function M.offer(snap, query)
    if snap.self.ghost or snap.self.class == "NONE" or snap.self.grouping ~= true or #(snap.group or {}) > 0 then
        return nil
    end
    if offers[snap] ~= nil then
        return offers[snap] or nil
    end
    local origin = snap
    snap         = score.recovered(snap, query)
    local mine   = solo(snap, query)
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
    local best = nil
    for rank, entry in ipairs(nearby) do
        if rank > combat.party.scan then
            break
        end
        local other = M.member(entry.profile)
        local plan  = ground.party({ snap, other }, query, "SUPPLY")
        if plan ~= nil and M.feasible(snap, query, plan, { snap, other }) == nil then
            plan = nil
        end
        if plan ~= nil then
            local gain    = (plan.peak or plan.rate) - mine
            local partner = (plan.peak or plan.rate) - solo(other, query)
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

local held = {}

function M.forget()
    for name in pairs(held) do
        held[name] = nil
    end
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
        local team = { snap }
        for _, member in ipairs(members) do
            table.insert(team, member)
        end
        local commit = #members == 0 and snap.record.memory.solo or snap.record.memory.commit
        local parts  = { (snap.now or 0) // combat.rerank, tostring(commit), table.concat(danger.shunned(snap, query),
            ",") }
        for _, member in ipairs(team) do
            table.insert(parts, score.signature(member, ""))
            table.insert(parts, tostring(member.self.ghost))
            table.insert(parts, tostring(ground.emptied(member)))
        end
        local key  = table.concat(parts, "|")
        local kept = held[snap.self.name]
        local plan = nil
        if kept ~= nil and kept.key == key then
            plan = kept.plan
        else
            plan = commit ~= nil and ground.hold(team, query, commit) or nil
            plan = plan or ground.party(team, query, "SUPPLY")
            held[snap.self.name] = { key = key, plan = plan }
        end
        if plan ~= nil then
            local copy = {}
            for key, entry in pairs(plan) do
                copy[key] = entry
            end
            copy.gate = M.gate(snap, query, plan, team) or plan.floors[1].map
            plan      = copy
        end
        plans[snap] = plan or false
    end
    return plans[snap] or nil
end

function M.leader(snap)
    for _, member in ipairs(M.members(snap)) do
        if member.self.name == snap.leader then
            return member
        end
    end
    return nil
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

local function held_in_place(member)
    return member.pinned == true or member.task == "DEFEND"
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
    -- The leader sets the group's direction and does not walk back: while it stands on a hunting floor
    -- (of the plan or of the ground it is leaving) the group meets where it is. Comparing walk costs alone
    -- sent the leader back to a member when the two costs were close, although both had to go on to the
    -- leader's floor anyway. Only a member that cannot come (moves keep failing, or it is fighting) is
    -- fetched by the cost comparison below.
    local held = false
    for index = 2, #team do
        held = held or held_in_place(team[index])
    end
    if held == false and snap.world.x ~= nil and ground.floor_at(snap.world.map) ~= nil then
        return { name = snap.self.name, map = snap.world.map, x = snap.world.x, y = snap.world.y, blocked = 0, cost = 0 }
    end
    -- A leader outside the ground goes in to the crew already hunting there; the crew does not walk
    -- out to the leader (it left the floor for the leader's shop and the group spent the run walking).
    local inside = {}
    for index = 2, #team do
        if ground.floor_at(team[index].world.map) ~= nil and team[index].world.x ~= nil then
            table.insert(inside, team[index])
        end
    end
    local candidates = #inside > 0 and inside or team
    local prices = {}
    for index, member in ipairs(team) do
        local worth = value.baseline(member, query)
        prices[index] = { worth = worth.value, loss = danger.loss(member, query, worth) }
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
                        if other ~= member and other ~= candidate and held_in_place(other) == false and
                            other.world.map == member.world.map and other.world.x ~= nil and member.world.x ~= nil and
                            math.abs(other.world.x - member.world.x) + math.abs(other.world.y - member.world.y) <=
                            combat.adjacent then
                            table.insert(companions, score.rested(other, ground.mode(other)))
                        end
                    end
                    local walk = walk_cost(member, query, target, prices[index].worth, prices[index].loss, companions)
                    if walk > 0 and (held_in_place(member) or walk == math.huge) then
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
        held = held or (member.self.ghost == false and held_in_place(member))
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
