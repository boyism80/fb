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
-- their crew at the rally for good (offline profile trio seed 12, 125-300s, run 13:15).
function M.pinned(snap)
    local blocked = snap.record.memory.blocked
    return M.stuck(snap, snap.pace.swing or combat.swing) and blocked.walls ~= true and blocked.detail ~= "CHARACTER"
end

-- Back in the ground since the servicing began: I last entered it after the servicing began, and on a hunting floor
-- whose entry is not noted yet I am entering it now, as the entry is noted only after the decision (decide.entered).
-- Counting only the entry noted before, a poet that followed its warrior back in after servicing cast a curse
-- first, then took its servicing as unfinished and walked out to town and back (offline WA+PO seeds 0-2, 375-392s).
function M.returned(snap)
    local memory = snap.record.memory
    local at     = memory.joined
    if ground.floor_at(snap.world.map) ~= nil then
        at = memory.entry ~= nil and memory.entry.at or snap.now
    end
    return memory.servicing ~= nil and at ~= nil and at > memory.servicing.at
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
        -- Pinned means my last walk failed and I have not moved since, whatever I do now. Posted only while
        -- walking, a poet rooted beside its warrior healed and then waited for it to cross, looked free and was
        -- left a floor behind (offline profile pair rooted healer seed 2, 273.9s); a member drinking in a shop is
        -- not fetched for it, as fetching is for the hunting floors (hunt stage). It holds beside the crew too:
        -- posted only once apart, a rogue rooted four tiles from its warrior at the entrance looked free, and the
        -- warrior and poet went in and waited inside without it (offline profile trio rooted seed 1, 204-262s).
        pinned    = grouped and M.pinned(snap) or nil,
        walled    = grouped and snap.world.x ~= nil and M.walled(snap, query) or nil,
        peril     = grouped and M.peril(snap, query) or nil,
        dealt     = grouped and require("brain.policies.battle").recent(snap.record.memory.dealt, snap.now) or nil,
        fallen    = snap.record.death ~= nil and
            { map = snap.record.death.map, at = snap.now - (snap.record.death.age or 0) } or nil,
        going     = decision ~= nil and decision.kind == ACTION.MOVE and decision.dest ~= nil and decision.dest.x ~= nil and
            { map = decision.dest.map, x = decision.dest.x, y = decision.dest.y } or nil,
        heading   = decision ~= nil and decision.kind == ACTION.MOVE and decision.dest ~= nil and
            (decision.dest.maps or { decision.dest.map }) or nil,
        arrives   = arrival(snap, query, decision),
        battle    = grouped and snap.record.memory.battle or nil,
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

local held  = {}
local gates = {}

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
            -- The entrance is where the group came in from, so it is taken while outside and kept inside: worked
            -- out from a floor, the way to the first floor is empty and the entrance came out as the floor itself,
            -- and a poet leader leaving 91 for supplies walked to where it stood for 60s while its rogue fought
            -- alone a floor deeper (offline RO+PO seed 2, 563-622s).
            local inside = false
            for _, floor in ipairs(plan.floors) do
                inside = inside or floor.map == snap.world.map
            end
            local kept = gates[snap.self.name]
            if inside and kept ~= nil and kept.name == plan.name then
                copy.gate = kept.map
            else
                copy.gate = M.gate(snap, query, plan, team) or plan.floors[1].map
                if inside == false then
                    gates[snap.self.name] = { name = plan.name, map = copy.gate }
                end
            end
            plan = copy
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
            local blow  = score.capability(member, query).blow
            local entry = {
                name  = member.self.name,
                melee = blow.adjacent >= blow.ranged,
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
-- crewmate it cannot see. My damage counts only while I fight one of them: a poet healing under blows kills
-- nothing, and counted by the spell damage it could deal it looked able to end its own attackers.
function M.peril(snap, query)
    local battle    = require("brain.policies.battle")
    local fight     = require("brain.policies.combat")
    local attackers = {}
    local drain     = 0
    local total     = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and (mob.speed or 0) > 0 and battle.attacking(snap, mob) then
            local _, high = fight.spread(snap, mob)
            drain         = drain + high * 1000 / mob.speed
            total         = total + (mob.hp or 0) * (mob.ratio or 1)
            table.insert(attackers, mob.oid)
        end
    end
    if #attackers == 0 then
        return nil
    end
    local fought = (snap.record.memory.battle or {}).oid
    local dps    = 0
    for _, oid in ipairs(attackers) do
        if oid == fought then
            dps = score.capability(snap, query).dps.single
        end
    end
    return { drain = drain, hp = total, dps = dps, attackers = attackers }
end

-- The leader names one crewmate to save and the one who goes. Saving is worth the death chance it removes
-- times what that death costs, the chance counted as in a heal's rescue from the blows left after the heals
-- crewmates in sight give it, over the time its attackers live once the helper's opening blow and then its
-- sustained damage join (sustained damage alone ranked a warrior below its poet). Its own heals do not
-- count, as healing itself is what a poet under blows must stop doing to keep its crew alive. Going costs
-- the hunting the helper gives up on the way, priced by the crew's one plan. A rogue that kills its
-- attackers itself is worth little and a poet that cannot is worth its life: a poet under blows waited a
-- minute for a warrior held by its own attackers (offline stage supply seed 2, 165-210s).
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
                local loss     = danger.loss(member, query, worth)
                local attacker = nil
                for _, mob in ipairs(snap.world.mobs or {}) do
                    for _, oid in ipairs(peril.attackers or {}) do
                        attacker = attacker or (mob.oid == oid and mob or nil)
                    end
                end
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
                            local arrive = steps * ((helper.member.pace or {}).step or combat.walk) / 1000
                            local left   = math.max(0, peril.hp - peril.dps * arrive - burst)
                            local ended  = arrive + left / (peril.dps + dps)
                            local saved  = (risk - math.exp(-member.self.hp / (drain * ended))) * loss
                            local net    = saved - arrive * worth.value
                            if net > 0 and (best == nil or net > best.value) then
                                best = { name = member.self.name, by = helper.member.self.name, value = net }
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

-- A rescuer's share in a mob: the rescue's worth for one beside the crewmate it saves, and for one beside me
-- that stands nearer that crewmate than I do, so the way is cut open before the crewmate's attackers.
function M.rescue_share(snap, query, mob)
    local victim, rescue = M.rescued(snap, query)
    if victim == nil or victim.world.map ~= snap.world.map or victim.world.x == nil or mob.x == nil then
        return 0
    end
    if math.abs(mob.x - victim.world.x) + math.abs(mob.y - victim.world.y) == 1 then
        return rescue.value
    end
    if math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
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
