local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local data     = require("brain.data.combat")
local decide   = require("brain.decide")
local ground   = require("brain.policies.ground")
local hunting  = require("brain.data.hunting")
local party    = require("brain.policies.party")
local items    = require("brain.data.items")
local recovery = require("brain.policies.recovery")
local spells   = require("brain.data.spells")
local driver   = require("sim.world.driver")
local facts    = require("sim.facts")
local roster   = require("sim.world.roster")
local suite    = require("sim.world.suite")

local records = nil
local mind    = nil

local function shared()
    records = records or require(roster.team.data)
    mind    = mind or brain.new()
    return records, mind
end

local ONLY = os.getenv("UNIT_ONLY")

local function skipped(name)
    return ONLY ~= nil and string.find(name, ONLY, 1, true) == nil
end

local function report(name, passed, detail)
    log("debug",
        string.format("[unit] %s %s%s", passed and "PASS" or "FAIL", name, passed and "" or " : " .. tostring(detail)))
    return passed
end

local function mob_named(name, x, y, oid)
    for _, mob in pairs(hunting.mobs) do
        if mob.name == name then
            local copy = {}
            for key, value in pairs(mob) do
                copy[key] = value
            end
            copy.x, copy.y, copy.oid, copy.ratio = x, y, oid, 1
            return copy
        end
    end
    error("no mob " .. name)
end

local function state(name, changes)
    local list, brain_ = shared()
    local base         = driver.prepare(suite.profile(list, name, 100000, brain_), roster.team)
    base.bag           = driver.stocked(base.bag.capacity)
    base.spells        = driver.versed(base.spells or {})
    base.record        = { memory = { serviced = 0 }, failures = {} }
    base.now           = 1000
    base.party         = {}
    base.group         = {}
    base.buffs         = {}
    base.pace          = { swing = data.swing, step = data.walk }
    base.world.x       = 10
    base.world.y       = 10
    base.world.mobs    = {}
    base.world.items   = {}
    base.self.hp       = base.self.max_hp
    base.self.mp       = base.self.max_mp
    for key, value in pairs(changes or {}) do
        base.self[key] = value
    end
    return base, facts.bind(brain_, base)
end

local function captured(name)
    local list, brain_ = shared()
    local found        = nil
    for _, record in ipairs(list.records) do
        if found == nil and record.snap.self.name == name then
            found = record.snap
        end
    end
    local base   = driver.prepare(found, roster.solo)
    base.bag     = driver.stocked(base.bag.capacity)
    base.now     = 1000
    base.party   = {}
    base.group   = {}
    base.pace    = { swing = data.swing, step = data.walk }
    base.self.hp = base.self.max_hp
    base.self.mp = base.self.max_mp
    return base, facts.bind(brain_, base)
end

local function surround(snap, name, count)
    local spots = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }
    for index = 1, count do
        table.insert(snap.world.mobs,
            mob_named(name, snap.world.x + spots[index][1], snap.world.y + spots[index][2], 100 + index))
    end
    return snap.world.mobs[1]
end

local function without(snap, name)
    local kept = {}
    for _, learned in ipairs(snap.spells) do
        if learned.name ~= name then
            table.insert(kept, learned)
        end
    end
    snap.spells = kept
end

local function walled(bound)
    return setmetatable({
        steps = function()
            return { false }
        end,
    }, { __index = bound })
end

local function apart_floors(me, mate, changes)
    local first, seen = state(me)
    local team        = state(mate)
    first.party       = { party.profile(team, seen, nil) }
    first.group       = { team.self.name, first.self.name }
    first.leader      = first.self.name
    local plan        = party.plan(first, seen)
    local snap, bound = state(me)
    local crew        = state(mate)
    crew.world.map    = plan.floors[1].map
    snap.world.map    = plan.floors[#plan.floors].map
    snap.party        = { party.profile(crew, bound, nil) }
    for key, value in pairs(changes or {}) do
        snap.party[1][key] = value
    end
    snap.group                = { crew.self.name, snap.self.name }
    snap.leader               = snap.self.name
    snap.record.memory.commit = plan.name
    snap.buffs                = {}
    for _, learned in ipairs(snap.spells) do
        local spell = spells.spells[learned.name]
        if spell ~= nil and spell.role == "BUFF" then
            table.insert(snap.buffs, learned.name)
        end
    end
    return snap, bound
end

local function after_flight(me, lead)
    local head, seen           = state(lead)
    local mate                 = state(me)
    head.party                 = { party.profile(mate, seen, nil) }
    head.group                 = { head.self.name, mate.self.name }
    head.leader                = head.self.name
    local plan                 = party.plan(head, seen)
    local profile              = party.profile(head, seen, nil)
    local snap, bound          = state(me)
    snap.party                 = { profile }
    snap.group                 = { head.self.name, snap.self.name }
    snap.leader                = head.self.name
    snap.record.memory.fleeing = { map = plan.floors[1].map, name = "x", at = snap.now - 9000, ends = snap.now - 5000 }
    snap.record.memory.joined  = snap.now - 60000
    snap.buffs                 = {}
    for _, learned in ipairs(snap.spells) do
        local spell = spells.spells[learned.name]
        if spell ~= nil and spell.role == "BUFF" then
            table.insert(snap.buffs, learned.name)
        end
    end
    return snap, bound, profile, plan
end

local function leave_by(name, walk, trapped, drop)
    local snap, bound = state(name)
    local go = require("brain.trees.go")
    for _, spell in ipairs(drop or {}) do
        without(snap, spell)
    end
    if trapped then
        snap.self.struck = 0
        snap.record.memory.spot = { map = snap.world.map, x = snap.world.x, y = snap.world.y, at = 0 }
    end
    local options = {}
    for index = 1, #go.jumps(snap, false) do
        table.insert(options, { index = index, cost = 30, dest = 29 })
    end
    local mock = setmetatable({
        leaps = function()
            return { walk = walk, options = options }
        end,
    }, { __index = bound })
    local chosen = go.choose(snap, mock, { maps = { 29 }, leaving = true },
        { closed = {}, perilous = {}, shunned = {}, spent = {} })
    return chosen ~= nil and chosen.name or "WALK"
end

local function threatened_walk(width, height, setup)
    local enums  = require("sim.world.runtime").enums()
    local open   = string.rep("\0", width * height)
    local map    = require("sim.world.map").new(1,
        { width = width, height = height, blocked = open, collisions = open }, enums)
    local threat = { x = setup.from[1], y = setup.from[2], data = { id = 7, speed = setup.speed } }
    map:enter(threat)
    local world    = {
        map = map,
        mobs = { [1] = threat },
        characters = {},
        enums = enums,
        move = function(self, object, direction)
            local allowed, x, y = self.map:passable(object.x, object.y, direction, object)
            if allowed ~= true then
                return false
            end
            self.map:shift(object, x, y)
            return true
        end,
    }
    local walker   = setmetatable({
        active = world,
        me = { x = 0, y = height // 2, hp = 100, dead = false },
        clock = 0,
        ends = 60000,
        pace = { step = 250 },
        memory = {},
        session = { group = function() return nil end, profiles = {} },
    }, { __index = driver.Actor })
    walker.closest = math.huge
    walker.advance = function(self, duration)
        self.clock = self.clock + duration
        if setup.at ~= nil and self.clock >= setup.at and threat.x ~= setup.to[1] then
            map:shift(threat, setup.to[1], setup.to[2])
        end
        self.closest = math.min(self.closest, math.abs(self.me.x - threat.x) + math.abs(self.me.y - threat.y))
    end
    return walker, threat
end

local function dogged(snap, bound, steps)
    local fired             = {}
    local dog               = require("sim.world.watch")
    local actor             = {
        clock = 0,
        me = { stats = { kills = 1, exp = 0 } },
        session = { actors = {} },
        violate = function(_, rule)
            fired[rule] = true
        end,
    }
    actor.session.actors[1] = actor
    for _, step in ipairs(steps) do
        actor.clock = step.at
        if step.change ~= nil then
            step.change(snap, actor)
        end
        dog.check(actor, snap, bound, step.decision, step.path)
    end
    return fired
end

local function repeated(count, spacing, decision, path, change)
    local steps = {}
    for index = 1, count do
        table.insert(steps, { at = index * spacing, decision = decision, path = path, change = change })
    end
    return steps
end

local function recorded(name)
    local _, brain_ = shared()
    local module = "sim.data.cases." .. name
    package.loaded[module] = nil
    local snap = require(module)
    return snap, facts.bind(brain_, snap)
end

local function as_healer(snap, bound)
    local poet  = snap.party[1]
    local world = {}
    for key, value in pairs(snap.world) do
        world[key] = value
    end
    world.x, world.y = poet.world.x, poet.world.y
    local view = {}
    for key, value in pairs(snap) do
        view[key] = value
    end
    view.self, view.bag, view.spells, view.buffs, view.pace, view.persona =
        poet.self, poet.bag, poet.spells, poet.buffs, poet.pace, poet.persona
    view.world                                                            = world
    view.record                                                           = { memory = { serviced = 0 }, failures = {} }
    view.party                                                            = { {
        self = snap.self,
        world = { map = snap.world.map, x = snap.world.x, y = snap.world.y },
        bag = snap.bag,
        spells = snap.spells,
        buffs = snap.buffs,
        pace = snap.pace,
        task = "DEFEND",
    } }
    local _, brain_                                                       = shared()
    local query                                                           = facts.bind(brain_, view)
    query.ally                                                            = function(name)
        return name == snap.self.name and { oid = 9001, percent = 100 * snap.self.hp / snap.self.max_hp } or nil
    end
    return view, query
end

local function swings_watched(snap, bound)
    local hits  = {}
    local actor = {
        clock = 0,
        me = { stats = { kills = 1, exp = 0 } },
        session = { actors = {} },
        violate = function(_, rule)
            hits[rule] = true
        end,
    }
    local check = require("sim.world.watch").check
    for _ = 1, 8 do
        actor.clock = actor.clock + 100
        check(actor, snap, bound, { task = "FIELD", kind = ACTION.ATTACK, key = "SWING" }, "strike")
    end
    return hits
end

local function swings_watched_dry(snap, bound)
    local hits  = {}
    local actor = {
        clock = 0,
        me = { stats = { kills = 1, exp = 0 }, mp = snap.self.mp, hp = snap.self.hp, max_hp = snap.self.max_hp },
        session = { actors = {} },
        violate = function(_, rule)
            hits[rule] = true
        end,
    }
    local check = require("sim.world.watch").check
    for index = 1, 40 do
        actor.clock = index * 1000
        check(actor, snap, bound, { task = "DEFEND", kind = ACTION.ATTACK, key = "SWING" }, "weigh")
    end
    return hits
end

local function any()
    return true
end

local NULL = {
    write = function()
    end,
    flush = function()
    end,
}

local function scene(name, members, options)
    if skipped(name) then
        return true
    end
    data.trace = true
    local list, brain_ = shared()
    local profiles = {}
    for _, member in ipairs(members) do
        local snap = suite.profile(list, member, 100000, brain_)
        local ready = {}
        for key, entry in pairs(snap) do
            ready[key] = entry
        end
        ready.bag       = options.bare and { items = {}, used = 0, capacity = snap.bag.capacity } or
            driver.stocked(snap.bag.capacity)
        ready.spells    = driver.versed(snap.spells or {})
        snap            = ready
        local overrides = { grouping = true }
        for key, value in pairs(options.changes ~= nil and options.changes[#profiles + 1] or {}) do
            overrides[key] = value
        end
        table.insert(profiles, {
            profile = snap,
            overrides = overrides,
            prepared = options.bare ~= true,
            rooted = options.rooted ~= nil and options.rooted[#profiles + 1] or nil
        })
    end
    local settings = setmetatable({
        seed = options.seed or 1,
        output = NULL,
        goal = options.goal,
        guard = true,
        halt = true,
        stall = 30000,
        bounce = 1000,
        sparse = options.sparse,
        arrange = options.arrange,
        drops = false,
        duration = (options.seconds or 120) * 1000,
    }, { __index = roster.team })
    local trace = os.getenv("UNIT_TRACE")
    if trace ~= nil then
        settings.acts   = true
        settings.output = io.open(trace, "wb")
    end
    local _, session = driver.team(brain_, profiles, settings, options.ground or {})
    local verdict    = session.verdict or "NO_GOAL"
    local detail     = {}
    for _, actor in ipairs(session.actors) do
        local rows = decide.offers[actor.base.self.name or ""] or {}
        local top  = rows[1] or {}
        table.insert(detail,
            string.format("%s map=%s (%s,%s) hp=%s mp=%s %s %s", actor.base.self.name, actor.active.map.id,
                actor.me.x, actor.me.y, actor.me.hp, actor.me.mp, tostring(top.task), tostring(top.path)))
    end
    return report(name, verdict == "PASS",
        string.format("%s t=%.0fs | %s", verdict, session.clock / 1000, table.concat(detail, " | ")))
end

local function case(name, body)
    return function()
        if skipped(name) then
            return true
        end
        require("brain.policies.party").forget()
        local ok, passed, detail = pcall(body)
        if ok == false then
            return report(name, false, passed)
        end
        return report(name, passed, detail)
    end
end

test_suite {
    name      = "Brain Unit Test",
    bot_count = 0,

    scenarios = {
        {
            parallel = {
                [0] = {
                    case("group plan does not assume zero mana", function()
                        local snap, bound = state("mage60")
                        local alone = ground.choose(snap, bound)
                        local party = ground.party({ snap }, bound, "SUPPLY")
                        return alone ~= nil and party ~= nil and alone.name == party.name,
                            tostring(alone and alone.name) .. " vs " .. tostring(party and party.name)
                    end),
                    case("a follower meets where the leader's rally point says, not at the leader", function()
                        local function decided(rally_at_self)
                            local snap, bound = recorded("poet_walks_in_alone")
                            snap.world.map, snap.world.x, snap.world.y, snap.world.mobs = 14, 137, 88, {}
                            snap.self.struck, snap.self.hp, snap.self.mp = nil, snap.self.max_hp, snap.self.max_mp
                            for _, member in ipairs(snap.party) do
                                if member.self.name == snap.leader then
                                    member.world, member.heading, member.task = { map = 93, x = 25, y = 23 }, nil,
                                        "FIELD"
                                    member.rally = rally_at_self and { name = snap.self.name, map = 14, x = 137, y = 88 } or
                                        { name = member.self.name, map = 93, x = 25, y = 23 }
                                end
                            end
                            local decision = decide.choose(snap, bound, nil, nil)
                            local dest = decision ~= nil and decision.dest or {}
                            return decision, dest
                        end
                        local stay, _ = decided(true)
                        local go, dest = decided(false)
                        return stay ~= nil and stay.kind == ACTION.HOLD and go ~= nil and go.kind == ACTION.MOVE and
                            (dest.map == 93 or (dest.maps or {})[1] ~= nil),
                            string.format("stay=%s go=%s", tostring(stay and stay.kind), tostring(go and go.kind))
                    end),
                    case("a leader deep inside stays and the member comes to it", function()
                        local snap, bound = recorded("leader_waits_deep")
                        snap.world.map, snap.world.x, snap.world.y, snap.world.mobs = 93, 25, 23, {}
                        snap.self.struck, snap.self.hp, snap.self.mp = nil, snap.self.max_hp, snap.self.max_mp
                        for _, member in ipairs(snap.party) do
                            if member.self.name == "sim1_64" then
                                member.world, member.heading, member.task = { map = 14, x = 137, y = 88 }, nil, "FIELD"
                            else
                                member.world, member.heading, member.task = { map = 93, x = 24, y = 23 }, nil, "FIELD"
                            end
                        end
                        local rally = party.rally(snap, bound)
                        local decision = decide.choose(snap, bound, nil, nil)
                        return rally ~= nil and rally.name == snap.self.name,
                            string.format("rally=%s cost=%s kind=%s", tostring(rally and rally.name),
                                tostring(rally and rally.cost),
                                tostring(decision and decision.kind))
                    end),
                    case("a leader waits only when the rally point is itself", function()
                        local together = require("brain.trees.together")
                        local snap, bound = recorded("leader_waits_in_town")
                        local rally = party.rally(snap, bound)
                        local step = together.to_rally(snap, bound)
                        if rally == nil then
                            return false, "premise: group not scattered"
                        end
                        local waits = step ~= nil and step.kind == ACTION.HOLD
                        return waits == (rally.name == snap.self.name),
                            string.format("rally=%s step=%s", tostring(rally.name),
                                tostring(step and step.kind))
                    end),
                    case("a member resting under attack is not pinned unless its moves fail", function()
                        local snap, bound = recorded("healer_rests_struck")
                        local span = snap.pace.swing or data.swing
                        if snap.self.struck == nil or snap.self.struck > span then
                            snap.self.struck = 0
                        end
                        if snap.party[1] == nil then
                            return false, "premise: no crew"
                        end
                        snap.party[1].world = { map = 3178, x = 79, y = 35 }
                        local resting = party.profile(snap, bound, { task = "REST", kind = ACTION.USE }).pinned
                        snap.record.memory.blocked = { at = snap.now, since = snap.now, map = snap.world.map }
                        local blocked = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.MOVE, dest = { maps = { snap.world.map } } }).pinned
                        return resting ~= true and blocked == true,
                            string.format("resting=%s blocked=%s", tostring(resting), tostring(blocked))
                    end),
                    case("a planned route follows where the bot stands on its map", function()
                        local value = require("brain.policies.value")
                        local snap, bound = recorded("leader_gate_flips")
                        snap.world.map, snap.world.x, snap.world.y, snap.world.mobs = 3178, 109, 7, {}
                        local view = setmetatable({
                            path = function(where, perilous, shunned, origin)
                                local x = (origin or snap.world).x
                                local hop = x < 150 and 22 or 3180
                                return {
                                    dest = 2263,
                                    x = 5,
                                    y = 5,
                                    cost = 40,
                                    path = { { dest = hop, x = x, y = 7, ax = 1, ay = 1 }, { dest = 2263, x = 3, y = 3, ax = 5, ay = 5 } }
                                }
                            end,
                        }, { __index = bound })
                        local worth = value.baseline(snap, view)
                        local near = value.trip(snap, view, { maps = { 2263 } }, worth)
                        snap.world.x, snap.world.y = 218, 107
                        local far = value.trip(snap, view, { maps = { 2263 } }, worth)
                        local function first(trip)
                            return trip ~= nil and trip.route ~= nil and trip.route.path ~= nil and
                                trip.route.path[1] ~= nil and trip.route.path[1].dest or nil
                        end
                        return first(near) == 22 and first(far) == 3180,
                            string.format("near=%s far=%s", tostring(first(near)), tostring(first(far)))
                    end),
                    case("mana need is zero without a mana skill", function()
                        local snap, bound = state("warrior63")
                        local plan = ground.choose(snap, bound)
                        if plan == nil then
                            return false, "premise: no ground"
                        end
                        snap.world.map = plan.floors[1].map
                        snap.spells = {}
                        local ok, need = pcall(require("brain.tasks.rest").mana_need, snap, bound)
                        return ok and need ~= nil and need >= 0, tostring(need)
                    end),
                    case("a failed reach query is not remembered as nowhere to go", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("rogue_no_food")
                        snap.world.x, snap.world.y = 7, 7
                        local target = { floors = { { map = 65001 } } }
                        local fails = true
                        local view = setmetatable({
                            reach = function(_, _, from)
                                if fails then
                                    return nil
                                end
                                return { [from.map] = true, [65001] = true }
                            end,
                        }, { __index = bound })
                        local during = g.reachable(snap, view, target)
                        fails = false
                        local after = g.reachable(snap, view, target)
                        return during == false and after == true,
                            string.format("during=%s after=%s", tostring(during), tostring(after))
                    end),
                    case("reachable grounds follow where the bot stands on its map", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("rogue_no_food")
                        local target = { floors = { { map = 65000 } } }
                        local view = setmetatable({
                            reach = function(_, _, from)
                                return from.x == 1 and { [from.map] = true } or { [from.map] = true, [65000] = true }
                            end,
                        }, { __index = bound })
                        snap.world.x, snap.world.y = 1, 1
                        local enclosed = g.reachable(snap, view, target)
                        snap.world.x = 2
                        local open = g.reachable(snap, view, target)
                        return enclosed == false and open == true,
                            string.format("enclosed=%s open=%s", tostring(enclosed), tostring(open))
                    end),
                    case("replay snapshot", function()
                        local path = os.getenv("UNIT_REPLAY")
                        if path == nil then
                            return true, "no snapshot"
                        end
                        local trace = require("sim.trace")
                        local snap = dofile(path)
                        local _, brain_ = shared()
                        local bound = facts.bind(brain_, snap)
                        local nodes, decision = trace.nodes(require("brain.trees.main"), function(s, b)
                            return decide.choose(s, b, nil, nil)
                        end, snap, bound)
                        local out = io.open(os.getenv("UNIT_REPLAY_OUT") or (path .. ".replay.txt"), "w")
                        out:write("decision " .. trace.described(decision) .. "\n")
                        for _, line in ipairs(nodes) do
                            out:write(line .. "\n")
                        end
                        out:close()
                        return true, trace.described(decision)
                    end),
                    case("walker keeps off the tiles beside a threat, also after it steps into the way", function()
                        local results = {}
                        for _, setup in ipairs({ { from = { 4, 2 } }, { from = { 4, 0 }, to = { 4, 2 }, at = 500 } }) do
                            local walker, threat = threatened_walk(9, 5, setup)
                            local walked, reason = walker:walk({ map = 1, x = 8, y = 2, range = 0 }, { [7] = true })
                            table.insert(results, string.format("%s:%s:%d:%d,%d", tostring(walked), reason,
                                walker.closest, walker.me.x, walker.me.y))
                            if walked ~= true or walker.closest < 2 or walker.me.x ~= 8 or walker.me.y ~= 2 then
                                return false, table.concat(results, " ")
                            end
                        end
                        return true, table.concat(results, " ")
                    end),
                    case("an unbeatable aggressive kind is a threat to walk around, not for a ghost", function()
                        local danger      = require("brain.policies.danger")
                        local snap, bound = state("warrior10")
                        table.insert(snap.world.mobs, mob_named("까만소", 16, 10, 900))
                        local alive     = danger.threats(snap, bound)
                        snap.self.ghost = true
                        local ghost     = danger.threats(snap, bound)
                        return #alive == 1 and alive[1] == 432 and #ghost == 0,
                            string.format("alive=%d ghost=%d", #alive, #ghost)
                    end),
                    case("walker stops when a threat holds the only way and does not move", function()
                        local walker         = threatened_walk(9, 1, { from = { 4, 0 } })
                        local walked, reason = walker:walk({ map = 1, x = 8, y = 0, range = 0 }, { [7] = true })
                        return walked == false and reason == "THREATENED" and walker.me.x < 3,
                            string.format("%s:%s:%d", tostring(walked), reason, walker.me.x)
                    end),
                    case("walker waits for a threat holding the only way to move off, then goes on", function()
                        local walker         = threatened_walk(15, 1,
                            { from = { 5, 0 }, to = { 12, 0 }, at = 1000, speed = 1000 })
                        local walked, reason = walker:walk({ map = 1, x = 4, y = 0, range = 0 }, { [7] = true })
                        return walked == true and walker.me.x == 4 and walker.clock >= 1000,
                            string.format("%s:%s:%d:%d", tostring(walked), reason, walker.me.x, walker.clock)
                    end),
                    case("walker waits for a crewmate ahead in a corridor", function()
                        local enums = require("sim.world.runtime").enums()
                        local function walk(leaves)
                            local map   = require("sim.world.map").new(1,
                                { width = 3, height = 1, blocked = "\0\0\0", collisions = {} }, enums)
                            local world = { map = map, characters = {}, enums = enums }
                            local mate  = { x = 1, y = 0 }
                            map:enter(mate)
                            world.characters[mate] = true
                            local walker = setmetatable(
                                {
                                    active = world,
                                    me = { x = 0, y = 0 },
                                    clock = 0,
                                    ends = 60000,
                                    pace = { step = 250 },
                                    session = { group = function() return nil end }
                                },
                                { __index = driver.Actor })
                            walker.advance = function(self, duration)
                                self.clock = self.clock + duration
                                if leaves ~= nil and self.clock >= leaves then
                                    map:shift(mate, 2, 0)
                                end
                            end
                            walker.me.dead = false
                            return walker:yielded(enums.DIRECTION.RIGHT, 2), walker.clock
                        end
                        local moved, waited = walk(500)
                        local stuck, gave = walk(nil)
                        return moved and waited == 500 and stuck == false and gave == 500,
                            string.format("moved=%s waited=%d stuck=%s gave=%d", tostring(moved), waited, tostring(stuck),
                                gave)
                    end),
                    case("imminent damage counts only worn protection", function()
                        local bare = state("poet65")
                        surround(bare, "양철인형", 1)
                        local worn = state("poet65")
                        worn.buffs = { "보호" }
                        surround(worn, "양철인형", 1)
                        local attackers = { { mob = bare.world.mobs[1], count = 1 } }
                        local left, right = combat.imminent(bare, attackers), combat.imminent(worn, attackers)
                        return left > right * 1.5, string.format("bare=%.0f worn=%.0f", left, right)
                    end),
                    case("imminent damage covers one critical back attack", function()
                        local snap = state("poet65")
                        surround(snap, "양철인형", 2)
                        local need = combat.mitigate(270, snap.self.phydef or 0, 0) * data.critical.factor * 2
                        local got  = battle.closing(snap)
                        return got >= need, string.format("closing=%.0f need=%.0f", got, need)
                    end),
                    case("no charge when the hp cost leaves less than the next blow", function()
                        local snap, bound = state("poet65", { hp = 1375, mp = 363 })
                        local mob         = surround(snap, "양철인형", 2)
                        local pick        = battle.choose(snap, bound, mob, true)
                        return pick ~= nil and pick.key ~= "CHARGE", tostring(pick and pick.key)
                    end),
                    case("charge when low on mana with safe hp", function()
                        local snap, bound = state("poet65", { mp = 35 })
                        local mob         = surround(snap, "목각인형", 1)
                        local pick        = battle.choose(snap, bound, mob, true)
                        return pick ~= nil and pick.key == "CHARGE", tostring(pick and pick.key)
                    end),
                    case("heal instead of attacking when hp is low and mana is there", function()
                        local snap, bound = state("poet65", { hp = 850, mp = 423 })
                        without(snap, "혼마술")
                        local mob  = surround(snap, "양철인형", 1)
                        local pick = battle.choose(snap, bound, mob, true)
                        return pick ~= nil and pick.kind == "HEAL", tostring(pick and pick.key)
                    end),
                    case("an alarm during a cooldown loot keeps the loot running", function()
                        local snap, bound = state("poet65")
                        local mob = surround(snap, "양철인형", 1)
                        snap.self.struck = 100
                        snap.self.casted = 50
                        snap.record.memory.battle = { map = snap.world.map, oid = mob.oid }
                        snap.world.items = { { oid = 900, x = snap.world.x, y = snap.world.y, names = { "사슴고기" } } }
                        local found = require("brain.tree").run(require("brain.trees.main"), snap, bound)
                        local fresh = decide.choose(snap, bound, nil, nil)
                        local kept = decide.choose(snap, bound, "LOOT", nil)
                        return found ~= nil and found.name ~= "LOOT" and fresh ~= nil and fresh.task == "LOOT" and
                            kept == nil,
                            string.format("found=%s fresh=%s kept=%s", tostring(found and found.name),
                                tostring(fresh and fresh.task), tostring(kept and kept.task))
                    end),
                    case("a still retaliating neighbour and an old blow do not make me flee", function()
                        local function commoner(struck, target)
                            local snap, bound                                              = state("warrior10")
                            snap.self.class, snap.self.level                               = "NONE", 5
                            snap.self.max_hp, snap.self.hp, snap.self.max_mp, snap.self.mp = 43, 43, 21, 21
                            snap.gear, snap.spells, snap.bag.items                         = {}, {}, {}
                            local stag                                                     = mob_named("숫사슴",
                                snap.world.x, snap.world.y - 1, 101)
                            local squirrel                                                 = mob_named("다람쥐",
                                snap.world.x, snap.world.y + 1, 102)
                            snap.world.mobs                                                = { stag, squirrel }
                            snap.record.memory.battle                                      = {
                                map = snap.world.map,
                                oid =
                                    target
                            }
                            snap.self.struck                                               = struck
                            return snap, bound, stag, squirrel
                        end
                        local snap, bound, stag, squirrel = commoner(120000, 102)
                        local decision = decide.choose(snap, bound, nil, nil)
                        local calm = decision ~= nil and decision.task ~= "ESCAPE" and
                            battle.attacking(snap, stag) == false and
                            battle.attacking(snap, squirrel) and battle.pressed(snap) == false
                        local hit, _, _, bitten = commoner(500, 102)
                        local provoked, _, angry = commoner(500, 101)
                        return calm and battle.pressed(hit) and battle.attacking(provoked, angry) and
                            battle.attacking(hit, bitten),
                            string.format("decision=%s stag=%s squirrel=%s pressed=%s hit=%s provoked=%s",
                                tostring(decision and decision.task), tostring(battle.attacking(snap, stag)),
                                tostring(battle.attacking(snap, squirrel)), tostring(battle.pressed(snap)),
                                tostring(battle.pressed(hit)), tostring(battle.attacking(provoked, angry)))
                    end),
                    case("a blocking mob is cleared only when I survive the fight", function()
                        local function survives(name)
                            local snap, bound = state("warrior10")
                            snap.self.class, snap.self.level = "NONE", 5
                            snap.self.max_hp, snap.self.hp, snap.self.max_mp, snap.self.mp = 50, 50, 35, 35
                            snap.gear, snap.spells, snap.bag.items = {}, {}, {}
                            local mob = mob_named(name, snap.world.x + 1, snap.world.y, 101)
                            snap.world.mobs = { mob }
                            local choice = battle.choose(snap, bound, mob)
                            return choice ~= nil and choice.survives
                        end
                        local rabbit, deer = survives("토끼"), survives("암사슴")
                        return rabbit and deer == false,
                            string.format("rabbit=%s deer=%s", tostring(rabbit), tostring(deer))
                    end),
                    case("an ambush never lands in a pocket I cannot walk out of", function()
                        local function offered(me, at)
                            local snap, bound = state("rogue50")
                            snap.world.map = 94
                            snap.world.x, snap.world.y = me[1], me[2]
                            snap.self.cloaked = true
                            local mob = mob_named("가재장", at[1], at[2], 101)
                            snap.world.mobs = { mob }
                            local before = data.trace
                            data.trace = true
                            local choice = battle.choose(snap, bound, mob, true)
                            data.trace = before
                            for _, entry in ipairs(choice ~= nil and choice.rivals or {}) do
                                if entry.key == "비영승보" then
                                    return true
                                end
                            end
                            return false
                        end
                        local open   = offered({ 10, 26 }, { 10, 25 })
                        local pocket = offered({ 2, 26 }, { 2, 25 })
                        local out    = offered({ 2, 24 }, { 2, 25 })
                        return open and pocket == false and out,
                            string.format("open=%s pocket=%s out=%s", tostring(open), tostring(pocket), tostring(out))
                    end),
                    case("an ambush is not offered when every tile it lands on is taken", function()
                        local function offered(crowd)
                            local snap, bound = state("rogue50")
                            snap.world.map = 94
                            snap.world.x, snap.world.y = 10, 26
                            snap.self.cloaked = true
                            local mob = mob_named("가재장", 10, 25, 101)
                            snap.world.mobs = { mob }
                            for index, spot in ipairs(crowd) do
                                table.insert(snap.world.mobs, mob_named("가재장", spot[1], spot[2], 200 + index))
                            end
                            local before = data.trace
                            data.trace = true
                            local choice = battle.choose(snap, bound, mob, true)
                            data.trace = before
                            for _, entry in ipairs(choice ~= nil and choice.rivals or {}) do
                                if entry.key == "비영승보" then
                                    return true
                                end
                            end
                            return false
                        end
                        local open  = offered({})
                        local one   = offered({ { 10, 24 }, { 11, 25 } })
                        local taken = offered({ { 10, 24 }, { 11, 25 }, { 9, 25 } })
                        return open and one and taken == false,
                            string.format("open=%s one=%s taken=%s", tostring(open), tostring(one), tostring(taken))
                    end),
                    case("healer does not walk to the crew at low hp", function()
                        local first, seen       = state("poet65")
                        local team              = state("warrior63")
                        first.party             = { party.profile(team, seen, nil) }
                        first.group             = { team.self.name, first.self.name }
                        first.leader            = team.self.name
                        local plan              = party.plan(first, seen)
                        local snap, bound       = state("poet65", { hp = 550, mp = 2200 })
                        local crew              = state("warrior63")
                        crew.world.map          = plan.floors[#plan.floors].map
                        snap.world.map          = plan.floors[1].map
                        snap.party              = { party.profile(crew, bound, nil) }
                        snap.party[1].plan      = plan
                        snap.group              = { crew.self.name, snap.self.name }
                        snap.leader             = crew.self.name
                        snap.self.struck        = snap.now
                        snap.record.memory.spot = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            at = snap
                                .now
                        }
                        surround(snap, "양철인형", 1)
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task ~= "FIELD",
                            tostring(pick and pick.task) .. " " .. tostring(pick and pick.kind)
                    end),
                    case("leader waits on the next floor while crew briefly recovers", function()
                        local snap, bound = apart_floors("rogue50", "poet65", { task = "REST" })
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.kind ~= ACTION.MOVE,
                            tostring(pick and pick.task) .. " " .. tostring(pick and pick.kind)
                    end),
                    case("leader goes back when crew is stuck", function()
                        local snap, bound = apart_floors("rogue50", "poet65", { task = "DEFEND", pinned = true })
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.kind == ACTION.MOVE,
                            tostring(pick and pick.task) .. " " .. tostring(pick and pick.kind)
                    end),
                    case("surrounded healer with no way to win escapes", function()
                        local snap, bound = state("poet65", { hp = 947, mp = 1023 })
                        local kept = {}
                        for _, item in ipairs(snap.bag.items) do
                            if require("brain.policies.recovery").restored(snap, require("brain.data.items")[item.name] or {}, "hp") <= 0 then
                                table.insert(kept, item)
                            end
                        end
                        snap.bag = { items = kept, used = #kept, capacity = snap.bag.capacity }
                        surround(snap, "해골", 2)
                        table.insert(snap.world.mobs, mob_named("칼든해골", snap.world.x, snap.world.y + 1, 111))
                        table.insert(snap.world.mobs, mob_named("칼든해골", snap.world.x, snap.world.y - 1, 112))
                        snap.self.struck = 0
                        snap.record.memory.spot = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            at = snap
                                .now
                        }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("healthy bot with a winnable fight does not escape", function()
                        local snap, bound = state("warrior63")
                        surround(snap, "해골", 1)
                        snap.self.struck = 0
                        snap.record.memory.spot = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            at = snap
                                .now
                        }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task ~= "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("leaving uses the cardinal leap even when the gate is close", function()
                        local means = leave_by("poet65", 3, false)
                        return means == "비영사천문", means
                    end),
                    case("leaving without the cardinal leap walks when the gate is very close", function()
                        local means = leave_by("poet65", 3, false, { "비영사천문" })
                        return means == "WALK", means
                    end),
                    case("trapped without the cardinal leap does not walk", function()
                        local means = leave_by("poet65", 3, true, { "비영사천문" })
                        return means == "귀환" or means == "노란비서", means
                    end),
                    case("leaving without the cardinal leap and far from the gate uses a scroll", function()
                        local means = leave_by("poet65", 200, false, { "비영사천문" })
                        return means == "귀환" or means == "노란비서", means
                    end),
                    case("the swing cooldown runs from the sent swing, not from a remembered reply", function()
                        local snap               = state("rogue50")
                        snap.record.memory.swung = snap.now
                        local unsent             = combat.backswing(snap)
                        snap.self.swung          = 100
                        local sent               = combat.backswing(snap)
                        return unsent == 0 and sent == data.swing - 100,
                            string.format("unsent=%s sent=%s swing=%s", unsent, sent, data.swing)
                    end),
                    case("a ghost struck just before it died does not swing back", function()
                        local defend = require("brain.tasks.defend")
                        local snap, bound = state("warrior10")
                        surround(snap, "양철인형", 1).attack = "AGGRESSIVE"
                        snap.self.struck = 0
                        local alive = defend.attacked(snap, bound)
                        snap.self.ghost = true
                        local ghost = defend.attacked(snap, bound)
                        return alive == true and ghost == false, string.format("alive=%s ghost=%s", alive, ghost)
                    end),
                    case("the pure state cases decide as the state test expects", function()
                        local ok, passed = xpcall(require("integration.scenarios.brain.state.cases").pure,
                            debug.traceback, nil, 0)
                        return ok and passed, tostring(passed)
                    end),
                    case("a hunt never walks to loot on a warp tile nor retries a tried one", function()
                        local hunt   = require("brain.policies.hunt")
                        local fake   = {
                            items = function()
                                return {
                                    { oid = 5, names = { "도토리" }, x = 10, y = 0 },
                                    { oid = 6, names = { "도토리" }, x = 3, y = 3 },
                                    { oid = 7, names = { "도토리" }, x = 9, y = 9 },
                                }
                            end,
                            warp_at = function(_, _, x, y)
                                return x == 10 and y == 0
                            end,
                        }
                        local bot    = {
                            position = function() return { 10, 1 } end,
                            map = function() return 320 end,
                        }
                        local first  = hunt.drop(fake, bot, { "도토리" }, {})
                        local second = hunt.drop(fake, bot, { "도토리" }, { [tostring(first and first.oid)] = true })
                        return first ~= nil and first.oid == 6 and second ~= nil and second.oid == 7,
                            string.format("first=%s second=%s", tostring(first and first.oid),
                                tostring(second and second.oid))
                    end),
                    case("crew on another floor hears the escape", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.record.memory.entry = { roster = "floor", at = snap.now - 60000 }
                        snap.party[1].fleeing = {
                            map = snap.party[1].world.map,
                            name = "해골",
                            at = snap.now - 1000,
                            ends = snap.now + 3000
                        }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("crew busy past the notice window still hears the escape", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.record.memory.entry = { roster = "floor", at = snap.now - 60000 }
                        snap.party[1].fleeing = {
                            map = snap.party[1].world.map,
                            name = "해골",
                            at = snap.now - 9000,
                            ends = snap.now - 5000
                        }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("a crewmate that fell on my ground makes me escape", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.record.memory.entry = { roster = "floor", at = snap.now - 60000 }
                        snap.party[1].self = setmetatable({ ghost = true, hp = 0 }, { __index = snap.party[1].self })
                        snap.party[1].fallen = { map = snap.party[1].world.map, at = snap.now - 1000 }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("a crewmate that fell before I entered or on another ground is not my escape", function()
                        local escape              = require("brain.tasks.escape")
                        local snap                = apart_floors("rogue50", "poet65")
                        snap.record.memory.entry  = { roster = "floor", at = snap.now - 1000 }
                        snap.party[1].fallen      = { map = snap.party[1].world.map, at = snap.now - 9000 }
                        local earlier             = escape.crew_notice(snap)
                        local other               = apart_floors("rogue50", "poet65")
                        other.record.memory.entry = { roster = "floor", at = other.now - 60000 }
                        other.party[1].fallen     = { map = 1793, at = other.now - 1000 }
                        local elsewhere           = escape.crew_notice(other)
                        return earlier == nil and elsewhere == nil,
                            string.format("earlier=%s elsewhere=%s", tostring(earlier ~= nil), tostring(elsewhere ~= nil))
                    end),
                    case("a member standing to drink is not pinned, one failing to move is", function()
                        local snap, bound          = apart_floors("rogue50", "poet65")
                        snap.record.memory.spot    = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            at =
                                snap.now - 5000
                        }
                        snap.record.memory.blocked = { map = snap.world.map, at = snap.now - 1000 }
                        local drinking             = party.profile(snap, bound,
                            { task = "REST", kind = ACTION.USE, item = "동동주" })
                        local walking              = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.MOVE, dest = { maps = { snap.world.map } } })
                        return drinking.pinned == nil and walking.pinned == true,
                            string.format("drinking=%s walking=%s", tostring(drinking.pinned), tostring(walking.pinned))
                    end),
                    case("a member outside waits at the entrance, not where the leader stands", function()
                        local snap, bound                          = state("warrior63")
                        local lead, seen                           = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 22, 14, 108
                        lead.world.map, lead.world.x, lead.world.y = 22, 60, 150
                        lead.group                                 = { lead.self.name, snap.self.name }
                        lead.leader                                = lead.self.name
                        lead.record.memory.commit                  = "국내해골굴"
                        snap.party                                 = { party.profile(lead, seen, nil) }
                        snap.party[1].task                         = "FIELD"
                        snap.party[1].plan                         = {
                            name = "국내해골굴",
                            rate = 1,
                            floors = { { map = 420, targets = {} }, { map = 421, targets = {} } },
                            gate = 22
                        }
                        snap.group                                 = { lead.self.name, snap.self.name }
                        snap.leader                                = lead.self.name
                        snap.record.memory.commit                  = "국내해골굴"
                        snap.buffs                                 = {}
                        for _, learned in ipairs(snap.spells) do
                            local spell = spells.spells[learned.name]
                            if spell ~= nil and spell.role == "BUFF" then
                                table.insert(snap.buffs, learned.name)
                            end
                        end
                        data.trace = true
                        local pick = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        data.trace = false
                        local dest = pick ~= nil and pick.dest or {}
                        return dest.map == 22 and dest.x ~= nil and math.abs(dest.x - 147) + math.abs(dest.y - 17) <= 1,
                            string.format("%s/%s dest=%s(%s,%s) maps=%s gate=%s path=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(dest.map), tostring(dest.x), tostring(dest.y),
                                table.concat(dest.maps or {}, ","),
                                tostring(snap.party[1].plan and snap.party[1].plan.gate),
                                tostring(rows[1] and rows[1].path))
                    end),
                    case("a poet short of mana for its strongest attack heals itself before charging under blows", function()
                        local snap, bound = captured("sim1_137")
                        snap.world.map, snap.world.x, snap.world.y = 5245, 25, 3
                        snap.self.hp, snap.self.mp = 2810, 597
                        snap.self.struck = 0
                        snap.world.mobs = { mob_named("자해골", 25, 4, 801), mob_named("지해골", 24, 3, 802), mob_named("흑해골",
                            26, 3, 803) }
                        local kept = {}
                        for _, item in ipairs(snap.bag.items) do
                            local effect = items[item.name]
                            if effect == nil or (effect.hp or 0) <= 0 then
                                table.insert(kept, item)
                            end
                        end
                        snap.bag     = { items = kept, used = #kept, capacity = snap.bag.capacity }
                        local charge = recovery.spells(snap).charge
                        local pick   = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "REST" and pick.kind == ACTION.CAST and charge ~= nil and
                            pick.slot ~= charge.slot,
                            string.format("%s/%s slot=%s charge=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind),
                                tostring(pick and pick.slot), tostring(charge and charge.slot))
                    end),
                    case("striking back counts only the danger the strike ends", function()
                        local aid                                  = require("brain.tasks.aid")
                        local defend                               = require("brain.tasks.defend")
                        local snap, bound                          = state("poet65")
                        local mate                                 = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        mate.world.map, mate.world.x, mate.world.y = 420, 25, 27
                        snap.self.hp                               = 2000
                        mate.self.hp                               = 1500
                        snap.self.struck                           = 0
                        snap.world.mobs                            = { mob_named("칼든해골", 24, 26, 601), mob_named("해골", 26,
                            26, 602),
                            mob_named("해골", 25, 25, 603), mob_named("칼든해골", 24, 27, 604), mob_named("해골", 25, 28, 605) }
                        snap.party                                 = { party.profile(mate, bound, nil) }
                        snap.party[1].task                         = "DEFEND"
                        snap.group                                 = { snap.self.name, mate.self.name }
                        snap.leader                                = snap.self.name
                        bound.ally                                 = function(name)
                            return name == mate.self.name and
                                { oid = 9001, percent = 100 * mate.self.hp / mate.self.max_hp } or nil
                        end
                        local care                                 = aid.value(snap, bound)
                        local strike                               = defend.value(snap, bound)
                        return care ~= nil and strike < care + 1,
                            string.format("care=%s strike=%s", tostring(care), tostring(strike))
                    end),
                    case("a fighting crewmate's heal fuels its hp strike without a shared battle", function()
                        local aid = require("brain.tasks.aid")
                        local function healed(shared)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 420, 25, 25
                            mate.world.map, mate.world.x, mate.world.y = 420, 25, 27
                            mate.self.hp                               = 1500
                            snap.world.mobs                            = { mob_named("해골", 25, 28, 701) }
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "DEFEND"
                            if shared then
                                snap.party[1].battle = { map = 420, oid = 701 }
                            end
                            snap.group  = { snap.self.name, mate.self.name }
                            snap.leader = snap.self.name
                            bound.ally  = function(name)
                                return name == mate.self.name and
                                    { oid = 9001, percent = 100 * mate.self.hp / mate.self.max_hp } or nil
                            end
                            return aid.worth("heal")(snap, bound)
                        end
                        local shared, unshared = healed(true), healed(false)
                        return shared ~= nil and unshared ~= nil and math.abs(shared - unshared) < 1e-6,
                            string.format("shared=%s unshared=%s", tostring(shared), tostring(unshared))
                    end),
                    case("a poet beside its resting warrior leaves the attacker to it, alone or out of sight it strikes back", function()
                        local function strike(percent, mate_map, mate_x)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                            mate.world.map, mate.world.x, mate.world.y = mate_map, mate_x or 25, 26
                            snap.self.hp, snap.self.mp                 = 2500, 1900
                            mate.self.hp                               = mate.self.max_hp * percent // 100
                            snap.self.struck                           = 0
                            snap.world.mobs                            = { mob_named("해골", 24, 26, 701), mob_named(
                                "칼든해골", 25, 25, 702),
                                mob_named("해골", 26, 26, 703), mob_named("칼든해골", 25, 27, 704) }
                            snap.record.memory.marks                   = {}
                            for _, mob in ipairs(snap.world.mobs) do
                                snap.record.memory.marks[tostring(mob.oid)] = { ["혼마술"] = snap.now }
                            end
                            snap.party          = { party.profile(mate, bound, nil) }
                            snap.party[1].task  = "REST"
                            snap.party[1].buffs = { "보호", "무장" }
                            snap.buffs          = { "보호", "무장" }
                            snap.group          = { snap.self.name, mate.self.name }
                            snap.leader         = snap.self.name
                            bound.ally          = function(name)
                                return name == mate.self.name and mate_map == 420 and
                                    math.abs(mate.world.x - snap.world.x) <= data.sight.x and
                                    { oid = 9001, percent = percent } or nil
                            end
                            return require("brain.tasks.defend").value(snap, bound)
                        end
                        local full, hurt, alone = strike(100, 420), strike(30, 420), strike(100, 22)
                        local far               = strike(30, 420, 37)
                        return full < 0 and hurt < 0 and alone >= 0 and far >= 0,
                            string.format("full=%s hurt=%s alone=%s far=%s", tostring(full), tostring(hurt),
                                tostring(alone),
                                tostring(far))
                    end),
                    case("a poet hit beside a low warrior heals instead of striking back", function()
                        local snap, bound                          = state("poet65")
                        local mate                                 = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        mate.world.map, mate.world.x, mate.world.y = 420, 25, 26
                        snap.self.hp, snap.self.mp                 = 2529, 2223
                        mate.self.hp                               = 900
                        snap.self.struck                           = 0
                        snap.world.mobs                            = { mob_named("해골", 25, 25, 601), mob_named("칼든해골", 26,
                            26, 602),
                            mob_named("칼든해골", 24, 26, 603), mob_named("해골", 25, 27, 604) }
                        for _, mob in ipairs(snap.world.mobs) do
                            mob.applied = { ["혼마술"] = true }
                        end
                        snap.party          = { party.profile(mate, bound, nil) }
                        snap.party[1].task  = "DEFEND"
                        snap.party[1].buffs = { "보호", "무장" }
                        snap.buffs          = { "보호", "무장" }
                        snap.group          = { snap.self.name, mate.self.name }
                        snap.leader         = snap.self.name
                        bound.ally          = function(name)
                            return name == mate.self.name and
                                { oid = 9001, percent = 100 * mate.self.hp / mate.self.max_hp } or nil
                        end
                        local pick          = decide.choose(snap, bound, nil, nil)
                        local strike        = require("brain.tasks.defend").value(snap, bound)
                        local care          = require("brain.tasks.aid").value(snap, bound)
                        return pick ~= nil and (pick.task == "AID" or pick.task == "REST") and strike < care,
                            string.format("%s/%s key=%s strike=%s care=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(pick and pick.key), tostring(strike),
                                tostring(care))
                    end),
                    case("a poet beside its fighting warrior leaves the attacker to it with nothing to aid", function()
                        local snap, bound                          = state("poet65")
                        local mate                                 = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        mate.world.map, mate.world.x, mate.world.y = 420, 25, 27
                        snap.self.struck                           = 0
                        snap.world.mobs                            = { mob_named("해골", 25, 25, 601), mob_named("칼든해골", 25,
                            28, 602) }
                        for _, mob in ipairs(snap.world.mobs) do
                            mob.applied = { ["혼마술"] = true }
                        end
                        snap.party          = { party.profile(mate, bound, nil) }
                        snap.party[1].task  = "DEFEND"
                        snap.party[1].buffs = { "보호", "무장" }
                        snap.buffs          = { "보호", "무장" }
                        snap.group          = { snap.self.name, mate.self.name }
                        snap.leader         = snap.self.name
                        bound.ally          = function(name)
                            return name == mate.self.name and { oid = 9001, percent = 100 } or nil
                        end
                        local strike        = require("brain.tasks.defend").value(snap, bound)
                        local pick          = decide.choose(snap, bound, nil, nil)
                        return strike < 0 and (pick == nil or pick.task ~= "DEFEND"),
                            string.format("strike=%s pick=%s/%s key=%s", tostring(strike), tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(pick and pick.key))
                    end),
                    case("a poet's mana is priced as its fighting warrior's fuel", function()
                        local aid = require("brain.tasks.aid")
                        local function fueling(task)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 420, 25, 25
                            mate.world.map, mate.world.x, mate.world.y = 420, 25, 27
                            snap.world.mobs                            = { mob_named("해골", 25, 28, 701) }
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = task
                            snap.group                                 = { snap.self.name, mate.self.name }
                            return aid.fueling(snap, bound)
                        end
                        local fighting, resting = fueling("DEFEND"), fueling("REST")
                        return fighting > 0 and resting == 0, string.format("fighting=%s resting=%s", fighting, resting)
                    end),
                    case("a poet hunting beside its warrior keeps its mana for the warrior's heals", function()
                        local function hunting(mate_map)
                            local snap, bound                          = state("poet65")
                            local lead, seen                           = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 420, 25, 9
                            lead.world.map, lead.world.x, lead.world.y = mate_map, 25, 8
                            snap.world.mobs                            = { mob_named("해골", 25, 13, 801) }
                            snap.world.mobs[1].applied                 = { ["혼마술"] = true }
                            lead.group                                 = { lead.self.name, snap.self.name }
                            lead.leader                                = lead.self.name
                            lead.record.memory.commit                  = "국내해골굴"
                            snap.party                                 = { party.profile(lead, seen, nil) }
                            snap.party[1].task                         = "FIELD"
                            snap.party[1].plan                         = {
                                name = "국내해골굴",
                                rate = 1,
                                floors = { { map = 420, targets = { "해골" } }, { map = 421, targets = { "해골" } } },
                                gate = 22
                            }
                            snap.group                                 = { lead.self.name, snap.self.name }
                            snap.leader                                = lead.self.name
                            snap.record.memory.commit                  = "국내해골굴"
                            snap.buffs                                 = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "BUFF" then
                                    table.insert(snap.buffs, learned.name)
                                end
                            end
                            local found = battle.target(snap, bound, function(mob)
                                return mob.oid == 801
                            end)
                            data.trace = true
                            local pick = decide.choose(snap, bound, nil, nil)
                            local rows = decide.offers[snap.self.name] or {}
                            data.trace = false
                            return found, pick, rows[1] and rows[1].path or "",
                                require("brain.tasks.aid").fueling(snap, bound,
                                    found and found.mob)
                        end
                        local found, pick, path, fuel = hunting(420)
                        local _, _, _, apart          = hunting(22)
                        local struck                  = path:match("strike$") ~= nil
                        return found ~= nil and found.choice.key == "화염주" and fuel > 0 and apart == 0 and struck == false,
                            string.format("key=%s fuel=%s apart=%s pick=%s/%s path=%s",
                                tostring(found and found.choice.key),
                                tostring(fuel), tostring(apart), tostring(pick and pick.task),
                                tostring(pick and pick.kind), path)
                    end),
                    case("a poet leading its warrior names it the vanguard and keeps with it", function()
                        local snap, bound                          = state("poet65")
                        local mate, seen                           = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 9
                        mate.world.map, mate.world.x, mate.world.y = 420, 25, 8
                        snap.world.mobs                            = { mob_named("해골", 25, 13, 802) }
                        snap.world.mobs[1].applied                 = { ["혼마술"] = true }
                        mate.group                                 = { snap.self.name, mate.self.name }
                        mate.leader                                = snap.self.name
                        snap.party                                 = { party.profile(mate, seen, nil) }
                        snap.party[1].task                         = "FIELD"
                        snap.group                                 = { snap.self.name, mate.self.name }
                        snap.leader                                = snap.self.name
                        snap.record.memory.commit                  = "국내해골굴"
                        snap.buffs                                 = {}
                        for _, learned in ipairs(snap.spells) do
                            local spell = spells.spells[learned.name]
                            if spell ~= nil and spell.role == "BUFF" then
                                table.insert(snap.buffs, learned.name)
                            end
                        end
                        data.trace   = true
                        local pick   = decide.choose(snap, bound, nil, nil)
                        local rows   = decide.offers[snap.self.name] or {}
                        data.trace   = false
                        local path   = rows[1] and rows[1].path or ""
                        local posted = party.profile(snap, bound, nil).vanguard
                        mate.party   = { party.profile(snap, bound, nil) }
                        mate.group   = { snap.self.name, mate.self.name }
                        mate.leader  = snap.self.name
                        local read   = party.vanguard_name(mate, seen)
                        return posted == mate.self.name and read == mate.self.name and pick ~= nil and
                            path:match("strike$") == nil and path:match("advance_crew") ~= nil,
                            string.format("posted=%s read=%s pick=%s/%s path=%s", tostring(posted), tostring(read),
                                tostring(pick and pick.task), tostring(pick and pick.kind), path)
                    end),
                    case("the vanguard is the melee hitter first, else the crewmate that stands the most blows", function()
                        local function front(me, other, changes)
                            local snap, bound = state(me)
                            local mate        = state(other)
                            for key, value in pairs(changes or {}) do
                                mate.self[key] = value
                            end
                            snap.party  = { party.profile(mate, bound, nil) }
                            snap.group  = { snap.self.name, mate.self.name }
                            snap.leader = snap.self.name
                            return party.vanguard_name(snap, bound), snap.self.name, mate.self.name
                        end
                        local warrior, _, fighter = front("poet65", "warrior63")
                        local rogue, _, stabber   = front("poet65", "rogue50")
                        local sturdy, _, caster   = front("poet65", "poet65", { name = "poet65_b", max_hp = 9000 })
                        return warrior == fighter and rogue == stabber and sturdy == caster,
                            string.format("warrior=%s rogue=%s sturdy=%s", tostring(warrior), tostring(rogue),
                                tostring(sturdy))
                    end),
                    case("a mob I cannot beat is passed alone but never when it keeps up or the pass could end me", function()
                        local danger = require("brain.policies.danger")
                        local function braving(changes, mate_at, steps)
                            local snap, bound                          = state("poet65")
                            snap.world.map, snap.world.x, snap.world.y = 420, 23, 22
                            local mob                                  = mob_named("칼든해골", 21, 22, 901)
                            mob.hp                                     = 100000000
                            for key, value in pairs(changes or {}) do
                                mob[key] = value
                            end
                            snap.world.mobs = { mob }
                            if mate_at ~= nil then
                                local mate = state("warrior63")
                                mate.world.map, mate.world.x, mate.world.y = 420, mate_at.x, mate_at.y
                                snap.party = { party.profile(mate, bound, nil) }
                                snap.group = { snap.self.name, mate.self.name }
                                snap.leader = snap.self.name
                            end
                            return battle.braves(snap, bound, mob, steps or 2), danger.hostile(snap, bound, mob)
                        end
                        local alone, hostile = braving(nil, nil)
                        local chaser         = braving({ speed = 200 }, nil)
                        local crushing       = braving({ damage = 800, least = 700 }, nil, 80)
                        return hostile and alone and chaser == false and crushing == false,
                            string.format("hostile=%s alone=%s chaser=%s crushing=%s", tostring(hostile), tostring(alone),
                                tostring(chaser), tostring(crushing))
                    end),
                    case("a leader kept off its way is fetched, one under blows names its rescue, else the crew meets at its vanguard", function()
                        local function rally(stuck, detail, hit)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 420, 23, 22
                            mate.world.map, mate.world.x, mate.world.y = 420, 10, 22
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "FIELD"
                            snap.group                                 = { snap.self.name, mate.self.name }
                            snap.leader                                = snap.self.name
                            snap.record.memory.commit                  = "국내해골굴"
                            snap.record.memory.spot                    = {
                                map = 420,
                                x = 23,
                                y = 22,
                                at = snap.now -
                                    5000
                            }
                            if stuck then
                                snap.record.memory.blocked = {
                                    map = 420,
                                    at = snap.now - 100,
                                    since = snap.now - 100,
                                    detail =
                                        detail
                                }
                            end
                            if hit then
                                snap.world.mobs  = { mob_named("칼든해골", 23, 21, 911) }
                                snap.self.struck = 0
                            end
                            local found = party.rally(snap, bound)
                            return found and found.name, snap.self.name, mate.self.name, party.rescue(snap, bound)
                        end
                        local kept, leader           = rally(true, "THREAT")
                        local crowded                = rally(true, "CHARACTER")
                        local struck, _, _, saved    = rally(false, nil, true)
                        local free, _, front, unhurt = rally(false)
                        local named                  = saved ~= nil and saved.name == leader and saved.by == front
                        return kept == leader and crowded == front and struck == front and named and unhurt == nil and
                            free == front,
                            string.format("kept=%s leader=%s crowded=%s struck=%s rescue=%s>%s free=%s front=%s",
                                tostring(kept), tostring(leader), tostring(crowded), tostring(struck),
                                tostring(saved and saved.by), tostring(saved and saved.name), tostring(free),
                                tostring(front))
                    end),
                    case("a leader keeps the entrance it came in from once inside the ground", function()
                        local function gate(map, x, y)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = map, x, y
                            mate.world.map, mate.world.x, mate.world.y = map, x, y + 1
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.group                                 = { snap.self.name, mate.self.name }
                            snap.leader                                = snap.self.name
                            snap.record.memory.commit                  = "국내해골굴"
                            local plan                                 = party.plan(snap, bound)
                            return plan and plan.gate, plan
                        end
                        local outside, plan = gate(22, 146, 20)
                        local inside        = gate(420, 25, 20)
                        local floor         = false
                        for _, entry in ipairs(plan and plan.floors or {}) do
                            floor = floor or entry.map == outside
                        end
                        return outside ~= nil and floor == false and inside == outside,
                            string.format("outside=%s inside=%s plan=%s", tostring(outside), tostring(inside),
                                tostring(plan and plan.name))
                    end),
                    case("a warrior short of mana under blows does not weigh a cure against a drink", function()
                        local snap, bound                          = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 20
                        snap.world.mobs                            = { mob_named("칼든해골", 25, 21, 951), mob_named("칼든해골",
                            24, 20, 952),
                            mob_named("칼든해골", 26, 20, 953) }
                        snap.self.struck                           = 0
                        snap.self.hp                               = snap.self.max_hp - 100
                        snap.self.mp                               = 48
                        local kept                                 = {}
                        for _, item in ipairs(snap.bag.items) do
                            if item.name == "동동주" then
                                table.insert(kept, item)
                            end
                        end
                        snap.bag.items = kept
                        data.trace = true
                        decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        data.trace = false
                        local path = rows[1] and rows[1].path or ""
                        return path ~= "" and path:find("cure_or_refuel", 1, true) == nil, path
                    end),
                    case("a member that followed the crew back in ends its servicing", function()
                        local function serviced(joined)
                            local snap, bound            = apart_floors("poet65", "warrior63")
                            snap.record.memory.servicing = { at = 1000, leave = true }
                            snap.record.memory.joined    = joined
                            decide.lap({ remember = function() end }, 1, snap, { kind = ACTION.HOLD, task = "FIELD" },
                                bound)
                            return snap.record.memory.servicing
                        end
                        local back    = serviced(2000)
                        local leaving = serviced(500)
                        return back == nil and leaving ~= nil,
                            string.format("back=%s leaving=%s", tostring(back ~= nil), tostring(leaving ~= nil))
                    end),
                    case("a warrior beside its poet still strikes back", function()
                        local snap, bound                          = state("warrior63")
                        local mate                                 = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        mate.world.map, mate.world.x, mate.world.y = 420, 25, 27
                        snap.self.struck                           = 0
                        snap.world.mobs                            = { mob_named("해골", 25, 25, 601) }
                        snap.party                                 = { party.profile(mate, bound, nil) }
                        snap.party[1].task                         = "AID"
                        snap.group                                 = { snap.self.name, mate.self.name }
                        snap.leader                                = snap.self.name
                        snap.buffs                                 = {}
                        for _, learned in ipairs(snap.spells) do
                            local spell = spells.spells[learned.name]
                            if spell ~= nil and spell.role == "BUFF" then
                                table.insert(snap.buffs, learned.name)
                            end
                        end
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "DEFEND",
                            string.format("%s/%s key=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(pick and pick.key))
                    end),
                    case("crew that entered after the escape ignores it", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.record.memory.entry = { roster = "floor", at = snap.now - 1000 }
                        snap.party[1].fleeing = {
                            map = snap.party[1].world.map,
                            name = "해골",
                            at = snap.now - 9000,
                            ends = snap.now - 5000
                        }
                        local notice = require("brain.tasks.escape").crew_notice(snap)
                        return notice == nil, tostring(notice and notice.name)
                    end),
                    case("an escape from another ground is not my escape", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.record.memory.entry = { roster = "floor", at = snap.now - 60000 }
                        snap.party[1].fleeing = { map = 14, name = "해골", at = snap.now - 1000, ends = snap.now + 3000 }
                        local notice = require("brain.tasks.escape").crew_notice(snap)
                        return notice == nil, tostring(notice and notice.name)
                    end),
                    case("a regroup ends once the bot enters the ground again", function()
                        local snap = after_flight("rogue50", "poet65")
                        local before = party.regrouping(snap)
                        snap.record.memory.joined = snap.now
                        return before == true and party.regrouping(snap) == false,
                            string.format("before=%s after=%s", tostring(before), tostring(party.regrouping(snap)))
                    end),
                    case("a regroup ends once the leader has gone back in after the flight", function()
                        local snap, _, lead = after_flight("rogue50", "poet65")
                        lead.joined = snap.now
                        return party.regrouping(snap) == false, tostring(party.regrouping(snap))
                    end),
                    case("a poet that heals itself with mana to spare needs no hp supplies", function()
                        local snap, bound = state("poet65")
                        if require("brain.policies.recovery").heal(snap) == nil then
                            return false, "premise: no heal skill"
                        end
                        local need = require("brain.tasks.supply").need_of(snap, bound, "hp")
                        return need == nil, tostring(need and need.usage)
                    end),
                    case("leaving the floor mid-hunt for supplies starts servicing for the group", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.record.memory.servicing, snap.record.memory.serviced = nil, snap.now - 60000
                        decide.lap({ remember = function() end }, 1, snap,
                            { kind = ACTION.MOVE, task = "SUPPLY", dest = { maps = { 14 } } }, bound)
                        local started = snap.record.memory.servicing
                        return started ~= nil and started.leave == true, tostring(started and started.at)
                    end),
                    case("servicing started by a crewmate makes a member on the floor walk out", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        snap.party[1].servicing = { at = snap.now - 1000, leave = true }
                        snap.record.memory.servicing, snap.record.memory.serviced = nil, snap.now - 60000
                        decide.lap({ remember = function() end }, 1, snap, { kind = ACTION.HOLD, task = "FIELD" }, bound)
                        local adopted = snap.record.memory.servicing
                        local gate = party.entry_gate(snap, bound, party.plan(snap, bound))
                        local pick = decide.choose(snap, bound, nil, nil)
                        local maps = pick ~= nil and pick.dest ~= nil and (pick.dest.maps or { pick.dest.map }) or {}
                        return adopted ~= nil and pick ~= nil and pick.kind == ACTION.MOVE and maps[1] == gate,
                            string.format("adopted=%s task=%s kind=%s dest=%s gate=%s", tostring(adopted ~= nil),
                                tostring(pick and pick.task), tostring(pick and pick.kind), tostring(maps[1]),
                                tostring(gate))
                    end),
                    case("a walker stopped on the leash waits until the crewmate moves", function()
                        local snap = apart_floors("rogue50", "poet65")
                        local mate = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 3, snap.world.y
                        local spot = { x = mate.world.x, y = mate.world.y }
                        snap.record.memory.leashed = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            crew = { [mate.self.name] = { x = spot.x, y = spot.y } }
                        }
                        local _, brain_ = shared()
                        local bound = require("sim.facts").bind(brain_, snap, {
                            ally = function()
                                return { x = spot.x, y = spot.y, percent = 100 }
                            end
                        })
                        local still = decide.choose(snap, bound, nil, nil)
                        spot.x = spot.x - 1
                        local moved = decide.choose(snap, bound, nil, nil)
                        return still ~= nil and still.leashed == true and (moved == nil or moved.leashed ~= true),
                            string.format("still=%s moved=%s", tostring(still and still.task),
                                tostring(moved and moved.task))
                    end),
                    case("a member closes in to the vanguard waiting on the leash", function()
                        local snap, bound                          = apart_floors("poet65", "rogue50")
                        local lead                                 = snap.party[1]
                        lead.world.map, lead.world.x, lead.world.y = snap.world.map, snap.world.x + 6, snap.world.y
                        lead.leashed, lead.task                    = true, "FIELD"
                        local pick                                 = decide.choose(snap, bound, nil, nil)
                        local dest                                 = pick ~= nil and pick.dest or {}
                        local steps                                = bound.steps(snap.world.map, lead.world.x,
                            lead.world.y, {
                                { x = dest.x or snap.world.x, y = dest.y or snap.world.y, range = 0 },
                                { x = snap.world.x, y = snap.world.y, range = 0 },
                            })
                        return pick ~= nil and pick.kind == ACTION.MOVE and type(steps[1]) == "number" and
                            type(steps[2]) == "number" and steps[1] < steps[2],
                            string.format("task=%s kind=%s dest=%s,%s walk=%s/%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(dest.x), tostring(dest.y), tostring(steps[1]),
                                tostring(steps[2]))
                    end),
                    case("the gap between casts walks the steps that fit toward a far crewmate", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = apart_floors("poet65", "rogue50")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + data.adjacent + 4,
                            snap.world.y
                        local step                                 = together.closer(snap, bound, combat.interval() - 100)
                        local short                                = together.closer(snap, bound, data.walk / 4)
                        local dest                                 = step ~= nil and step.dest or {}
                        local walks                                = bound.steps(snap.world.map, mate.world.x,
                            mate.world.y, {
                                { x = dest.x or snap.world.x, y = dest.y or snap.world.y, range = 0 },
                                { x = snap.world.x, y = snap.world.y, range = 0 },
                            })
                        return step ~= nil and step.kind == ACTION.MOVE and dest.steps == 1 and
                            type(walks[1]) == "number" and walks[1] < walks[2] and short == nil,
                            string.format("kind=%s steps=%s walk=%s/%s short=%s", tostring(step and step.kind),
                                tostring(dest.steps), tostring(walks[1]), tostring(walks[2]),
                                tostring(short and short.kind))
                    end),
                    case("off the ground the gap between casts waits at the entrance and never jumps", function()
                        local together                             = require("brain.trees.together")
                        local go                                   = require("brain.trees.go")
                        local snap, bound                          = apart_floors("poet65", "rogue50")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + data.adjacent + 4,
                            snap.world.y
                        local inside                               = together.closer(snap, bound, combat.interval() - 100)
                        local detour                               = { avoid = {}, closed = {} }
                        go.choose(snap, bound, inside ~= nil and inside.dest or {}, detour)
                        local stepped = #detour.jumps
                        local trip    = { avoid = {}, closed = {} }
                        go.choose(snap, bound, { maps = { 22 } }, trip)
                        local tripped = #trip.jumps
                        snap.world.map, mate.world.map = 22, 22
                        snap.world.x, snap.world.y = 146, 20
                        mate.world.x, mate.world.y = 48, 150
                        local outside = together.closer(snap, bound, combat.interval() - 100)
                        return inside ~= nil and stepped == 0 and tripped > 0 and outside == nil,
                            string.format("inside=%s jumps=%s trip=%s outside=%s", tostring(inside and inside.kind),
                                tostring(stepped), tostring(tripped), tostring(outside and outside.kind))
                    end),
                    case("a crewmate beside me or a mob beside me keeps the gap between casts still", function()
                        local together = require("brain.trees.together")
                        local snap, bound = apart_floors("poet65", "rogue50")
                        local mate = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x - 1, snap.world.y
                        local near = together.closer(snap, bound, combat.interval())
                        mate.world.x = snap.world.x + data.adjacent + 4
                        snap.world.mobs = { mob_named("칼든해골", snap.world.x - 1, snap.world.y, 501) }
                        local fighting = together.closer(snap, bound, combat.interval())
                        return near == nil and fighting == nil,
                            string.format("near=%s fighting=%s", tostring(near and near.kind),
                                tostring(fighting and fighting.kind))
                    end),
                    case("a healer between two crewmates stays between them and one off to a side walks in", function()
                        local spacing = require("brain.policies.spacing")
                        local function placed(x, y)
                            local snap, bound                                                     = apart_floors(
                                "poet65", "warrior63")
                            local rogue                                                           = state("rogue50")
                            rogue.world.map                                                       = snap.world.map
                            snap.party[1].world.map, snap.party[1].world.x, snap.party[1].world.y = snap.world.map, 4, 10
                            rogue.world.x, rogue.world.y                                          = 16, 10
                            table.insert(snap.party, party.profile(rogue, bound, nil))
                            table.insert(snap.group, rogue.self.name)
                            snap.world.x, snap.world.y = x, y
                            return spacing.place(snap, bound, 2)
                        end
                        local middle = placed(10, 9)
                        local side   = placed(22, 10)
                        return (middle == nil or math.abs(middle.x - 10) <= 1) and side ~= nil and side.x < 22,
                            string.format("middle=%s side=%s", middle and (middle.x .. "," .. middle.y) or "stay",
                                side and (side.x .. "," .. side.y) or "stay")
                    end),
                    case("the vanguard's walk is held by a crewmate that cannot move too", function()
                        local escort                               = require("brain.policies.escort")
                        local snap, bound                          = apart_floors("warrior63", "poet65")
                        local mate                                 = snap.party[1]
                        snap.world.x, snap.world.y                 = 5, 14
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, 5, 19
                        mate.pinned, mate.task                     = true, "FIELD"
                        local plan                                 = party.plan(snap, bound)
                        local name                                 = nil
                        for _, floor in ipairs(plan.floors) do
                            if floor.map == snap.world.map then
                                name = floor.targets[1]
                            end
                        end
                        snap.world.mobs = { mob_named(name, 12, 14, 777) }
                        local pick      = decide.choose(snap, bound, nil, nil)
                        local leash     = pick ~= nil and pick.dest ~= nil and pick.dest.leash or {}
                        local at        = (leash.at or {})[mate.self.name] or {}
                        local held      = false
                        for _, entry in ipairs(leash.names or {}) do
                            held = held or entry == mate.self.name
                        end
                        return held and at.x == 5 and at.y == 19 and leash.steps == escort.reach(),
                            string.format("kind=%s held=%s at=%s,%s steps=%s", tostring(pick and pick.kind),
                                tostring(held),
                                tostring(at.x), tostring(at.y), tostring(leash.steps))
                    end),
                    case("the leader sends help to the poet that cannot end its attackers, not the rogue that can", function()
                        local snap, bound                             = apart_floors("warrior63", "poet65")
                        local poet                                    = state("poet65")
                        local rogue                                   = state("rogue50")
                        snap.world.x, snap.world.y                    = 10, 10
                        poet.world.map, poet.world.x, poet.world.y    = snap.world.map, 6, 10
                        rogue.world.map, rogue.world.x, rogue.world.y = snap.world.map, 14, 10
                        surround(poet, "칼든해골", 3)
                        surround(rogue, "칼든해골", 3)
                        poet.self.struck, rogue.self.struck = 0, 0
                        rogue.record.memory.battle = { oid = rogue.world.mobs[1].oid, map = snap.world.map }
                        snap.group = { snap.self.name, poet.self.name, rogue.self.name }
                        poet.group, rogue.group = snap.group, snap.group
                        snap.party = { party.profile(poet, bound, nil), party.profile(rogue, bound, nil) }
                        local rescue = party.rescue(snap, bound)
                        return rescue ~= nil and rescue.name == poet.self.name,
                            string.format("rescue=%s>%s", tostring(rescue and rescue.by),
                                tostring(rescue and rescue.name))
                    end),
                    case("a rescuer under blows cuts through the mob in its way to the crewmate first", function()
                        local defend                               = require("brain.tasks.defend")
                        local snap, bound                          = apart_floors("warrior63", "poet65")
                        local poet                                 = state("poet65")
                        snap.world.x, snap.world.y                 = 10, 10
                        poet.world.map, poet.world.x, poet.world.y = snap.world.map, 6, 10
                        poet.world.mobs                            = { mob_named("칼든해골", 5, 10, 601), mob_named("칼든해골", 6,
                            9, 602),
                            mob_named("칼든해골", 6, 11, 603) }
                        poet.self.struck                           = 0
                        snap.group                                 = { snap.self.name, poet.self.name }
                        poet.group                                 = snap.group
                        snap.party                                 = { party.profile(poet, bound, nil) }
                        snap.world.mobs                            = { mob_named("칼든해골", 11, 10, 701), mob_named("칼든해골",
                            9, 10, 702) }
                        snap.self.struck                           = 0
                        local rescue                               = party.rescue(snap, bound)
                        local action                               = defend.strike_back(snap, bound)
                        return rescue ~= nil and rescue.by == snap.self.name and action ~= nil and action.oid == 702,
                            string.format("rescue=%s>%s oid=%s", tostring(rescue and rescue.by),
                                tostring(rescue and rescue.name), tostring(action and action.oid))
                    end),
                    case("a vanguard under blows with the crew meeting on it fights instead of standing", function()
                        local snap, bound = apart_floors("warrior63", "poet65")
                        surround(snap, "칼든해골", 1)
                        snap.self.struck = 0
                        local pick       = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.kind ~= ACTION.HOLD,
                            string.format("task=%s kind=%s", tostring(pick and pick.task), tostring(pick and pick.kind))
                    end),
                    case("a member that crossed first waits for the vanguard coming to its floor", function()
                        local together = require("brain.trees.together")
                        local function rally(heading)
                            local snap, bound     = apart_floors("poet65", "warrior63")
                            snap.party[1].task    = "FIELD"
                            snap.party[1].heading = heading and { snap.world.map } or nil
                            return together.to_rally(snap, bound)
                        end
                        local away   = rally(false)
                        local coming = rally(true)
                        return away ~= nil and away.kind == ACTION.MOVE and coming ~= nil and coming.kind == ACTION.HOLD,
                            string.format("away=%s coming=%s", tostring(away and away.kind),
                                tostring(coming and coming.kind))
                    end),
                    case("with no tile aside a member makes way one step ahead toward the crewmate's goal", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                        mate.task, mate.going                      = "FIELD",
                            { map = snap.world.map, x = snap.world.x - 6, y = snap.world.y }
                        local row                                  = snap.world.y
                        local corridor                             = setmetatable({
                            floor = function(_, _, y)
                                return y == row
                            end,
                            steps = function(_, x, y, goals)
                                local found = {}
                                for index, goal in ipairs(goals) do
                                    found[index] = goal.y == y and math.abs(goal.x - x) or false
                                end
                                return found
                            end,
                        }, { __index = bound })
                        local found                                = require("brain.tree").run(together.make_way, snap,
                            corridor)
                        local dest                                 = found ~= nil and found.action ~= nil and
                            found.action.dest or {}
                        return dest.x == snap.world.x - 1 and dest.y == snap.world.y and dest.steps == 1,
                            string.format("dest=%s,%s steps=%s", tostring(dest.x), tostring(dest.y), tostring(dest.steps))
                    end),
                    case("a member four tiles from the rally but a long walk round a wall walks to it", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = state("rogue50")
                        local lead                                 = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 92, 22, 14
                        lead.world.map, lead.world.x, lead.world.y = 92, 21, 17
                        snap.group                                 = { lead.self.name, snap.self.name }
                        snap.leader                                = lead.self.name
                        snap.party                                 = { party.profile(lead, bound, nil) }
                        snap.party[1].rally                        = { name = lead.self.name, map = 92, x = 21, y = 17 }
                        local found                                = together.to_rally(snap, bound)
                        local dest                                 = found ~= nil and found.dest or {}
                        return found ~= nil and found.kind == ACTION.MOVE and dest.range == 1,
                            string.format("kind=%s range=%s", tostring(found and found.kind), tostring(dest.range))
                    end),
                    case("a member at the only way out of a dead end makes way ahead, not past the crewmate", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = state("poet65")
                        local mate                                 = state("warrior63")
                        snap.world.map, snap.world.x, snap.world.y = 964, 3, 1
                        mate.world.map, mate.world.x, mate.world.y = 964, 2, 1
                        snap.group                                 = { mate.self.name, snap.self.name }
                        snap.leader                                = mate.self.name
                        snap.party                                 = { party.profile(mate, bound, nil) }
                        snap.party[1].task                         = "FIELD"
                        snap.party[1].going                        = { map = 964, x = 8, y = 5 }
                        local found                                = require("brain.tree").run(together.make_way, snap,
                            bound)
                        local dest                                 = found ~= nil and found.action ~= nil and
                            found.action.dest or {}
                        local ahead                                = (dest.x == 4 and dest.y == 1) or
                            (dest.x == 3 and dest.y == 2)
                        return ahead and dest.steps == 1,
                            string.format("dest=%s,%s steps=%s", tostring(dest.x), tostring(dest.y), tostring(dest.steps))
                    end),
                    case("a member whose moves keep failing beside its crew is posted pinned", function()
                        local snap, bound                          = state("rogue50")
                        local lead                                 = state("warrior63")
                        lead.world.map, lead.world.x, lead.world.y = snap.world.map, snap.world.x + 2, snap.world.y
                        snap.group                                 = { lead.self.name, snap.self.name }
                        snap.leader                                = lead.self.name
                        snap.party                                 = { party.profile(lead, bound, nil) }
                        snap.record.memory.spot                    = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            at = snap.now - 5000
                        }
                        snap.record.memory.blocked                 = {
                            map = snap.world.map,
                            at = snap.now - 100,
                            since = snap.now - 100
                        }
                        local walking                              = party.profile(snap, bound,
                            { kind = ACTION.MOVE, task = "FIELD" })
                        local drinking                             = party.profile(snap, bound,
                            { kind = ACTION.ITEM or ACTION.HOLD, task = "REST" })
                        return walking.pinned == true and drinking.pinned == nil,
                            string.format("walking=%s drinking=%s", tostring(walking.pinned), tostring(drinking.pinned))
                    end),
                    case("a crew inside meets a member that cannot move at the entrance outside", function()
                        local snap, bound                             = state("warrior63")
                        local poet                                    = state("poet65")
                        local rogue                                   = state("rogue50")
                        snap.world.map, snap.world.x, snap.world.y    = 91, 26, 25
                        poet.world.map, poet.world.x, poet.world.y    = 91, 27, 25
                        rogue.world.map, rogue.world.x, rogue.world.y = 14, 129, 43
                        snap.group                                    = { snap.self.name, poet.self.name, rogue.self
                            .name }
                        snap.leader                                   = snap.self.name
                        snap.party                                    = { party.profile(poet, bound, nil), party.profile(
                            rogue, bound, nil) }
                        snap.party[1].task                            = "FIELD"
                        snap.party[2].task, snap.party[2].pinned      = "FIELD", true
                        local rally                                   = party.rally(snap, bound)
                        return rally ~= nil and rally.name == rogue.self.name,
                            string.format("rally=%s", tostring(rally and rally.name))
                    end),
                    case("walker beside a threat that strikes it does not wait for it to move off", function()
                        local walker, threat = threatened_walk(9, 1, { from = { 1, 0 }, speed = 1000 })
                        walker.me.x = 0
                        local walked, reason = walker:walk({ map = 1, x = 8, y = 0, range = 0 }, { [7] = true })
                        return walked == false and reason == "THREATENED" and walker.clock == 0 and threat.x == 1,
                            string.format("%s:%s:clock=%d", tostring(walked), reason, walker.clock)
                    end),
                    case("a healer far from its hurt crewmate steps toward it between heals", function()
                        local snap = apart_floors("poet65", "rogue50")
                        local mate = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + data.adjacent + 4,
                            snap.world.y
                        mate.self.hp = mate.self.max_hp * 2 // 5
                        mate.task = "DEFEND"
                        snap.leader = mate.self.name
                        local _, brain_ = shared()
                        local bound = require("sim.facts").bind(brain_, snap, {
                            ally = function(name)
                                return name == mate.self.name and
                                    { oid = 9001, x = mate.world.x, y = mate.world.y, percent = 40 } or nil
                            end
                        })
                        snap.self.casted = nil
                        local ready = decide.choose(snap, bound, nil, nil)
                        if ready == nil or combat.casting(ready) == false then
                            return false, "premise: no cast when ready " .. tostring(ready and ready.task)
                        end
                        snap.self.casted = 100
                        local gap        = decide.choose(snap, bound, nil, nil)
                        local dest       = gap ~= nil and gap.dest or {}
                        local walks      = bound.steps(snap.world.map, mate.world.x, mate.world.y, {
                            { x = dest.x or snap.world.x, y = dest.y or snap.world.y, range = 0 },
                            { x = snap.world.x, y = snap.world.y, range = 0 },
                        })
                        return gap ~= nil and gap.kind == ACTION.MOVE and dest.steps == 1 and type(walks[1]) == "number" and
                            walks[1] < walks[2],
                            string.format("task=%s kind=%s steps=%s walk=%s/%s", tostring(gap and gap.task),
                                tostring(gap and gap.kind), tostring(dest.steps), tostring(walks[1]), tostring(walks[2]))
                    end),
                    case("crewmates are watched so their blows decide again during a walk", function()
                        local snap = apart_floors("poet65", "rogue50")
                        local seen = nil
                        require("brain.policies.alert").watch({
                            concern = function(_, _, value)
                                seen = value
                            end
                        }, nil, snap)
                        local mate = snap.party[1].self.name
                        return seen ~= nil and seen.crew ~= nil and seen.crew[mate] == true, tostring(seen and seen.crew)
                    end),
                    case("a member beside the vanguard crosses with it to the next floor", function()
                        local probe, probed = apart_floors("poet65", "rogue50")
                        local plan = party.plan(probe, probed)
                        local first, second = plan.floors[1].map, plan.floors[2] and plan.floors[2].map
                        local snap, bound = apart_floors("poet65", "rogue50")
                        local lead = snap.party[1]
                        snap.world.map, snap.world.x, snap.world.y, snap.world.mobs = first, 10, 10, {}
                        lead.world.map, lead.world.x, lead.world.y = first, 11, 10
                        lead.heading, lead.task = { second }, "FIELD"
                        local pick = decide.choose(snap, bound, nil, nil)
                        local maps = pick ~= nil and pick.dest ~= nil and (pick.dest.maps or { pick.dest.map }) or {}
                        return second ~= nil and pick ~= nil and pick.kind == ACTION.MOVE and maps[1] == second,
                            string.format("task=%s kind=%s dest=%s next=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(maps[1]), tostring(second))
                    end),
                    case("after a flight a member walks to the gate, not to the leader inside", function()
                        local snap, bound, lead, plan = after_flight("rogue50", "poet65")
                        lead.world.map, lead.world.x, lead.world.y = plan.floors[#plan.floors].map, 5, 5
                        lead.heading = nil
                        local gate = plan.gate
                        local pick = decide.choose(snap, bound, nil, nil)
                        local maps = pick ~= nil and pick.dest ~= nil and (pick.dest.maps or { pick.dest.map }) or {}
                        return pick ~= nil and pick.kind == ACTION.MOVE and maps[1] == gate,
                            string.format("task=%s kind=%s dest=%s gate=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(maps[1]), tostring(gate))
                    end),
                    case("after a flight a member at the gate waits for the leader coming out", function()
                        local snap, bound, lead, plan = after_flight("rogue50", "poet65")
                        local gate = plan.gate
                        snap.world.map = gate
                        lead.world.map, lead.world.x, lead.world.y = plan.floors[1].map, 5, 5
                        lead.heading = { gate }
                        local pick = decide.choose(snap, bound, nil, nil)
                        local maps = pick ~= nil and pick.dest ~= nil and (pick.dest.maps or { pick.dest.map }) or {}
                        return pick ~= nil and (pick.kind == ACTION.HOLD or maps[1] == gate),
                            string.format("task=%s kind=%s dest=%s gate=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(maps[1]), tostring(gate))
                    end),
                    case("after a flight a member at the gate follows the leader crossing in", function()
                        local snap, bound, lead, plan = after_flight("rogue50", "poet65")
                        local gate = plan.gate
                        snap.world.map = gate
                        lead.world.map, lead.world.x, lead.world.y = gate, snap.world.x + 1, snap.world.y
                        lead.heading = { plan.floors[1].map }
                        local pick = decide.choose(snap, bound, nil, nil)
                        local maps = pick ~= nil and pick.dest ~= nil and (pick.dest.maps or { pick.dest.map }) or {}
                        return pick ~= nil and pick.kind == ACTION.MOVE and maps[1] == plan.floors[1].map,
                            string.format("task=%s kind=%s dest=%s first=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(maps[1]),
                                tostring(plan.floors[1].map))
                    end),
                    case("healthy bot in a hard fight does not escape", function()
                        local snap, bound = state("poet65")
                        surround(snap, "해골", 4)
                        snap.self.struck = 0
                        snap.record.memory.spot = {
                            map = snap.world.map,
                            x = snap.world.x,
                            y = snap.world.y,
                            at = snap
                                .now
                        }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task ~= "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("watch: idle fires when nothing changes", function()
                        local snap, bound = state("poet65")
                        return dogged(snap, bound,
                                repeated(5, 20000, { task = "FIELD", kind = ACTION.HOLD }, "x>wait_crew")).idle == true,
                            "no idle"
                    end),
                    case("watch: long waiting fires", function()
                        local snap, bound = state("poet65")
                        local fired = dogged(snap, bound,
                            repeated(31, 10000, { task = "FIELD", kind = ACTION.HOLD }, "x>wait_leader",
                                function(s, actor)
                                    s.self.mp = actor.clock % 100
                                end))
                        return fired["waiting:wait_leader"] == true, "no waiting"
                    end),
                    case("watch: split fires", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        return dogged(snap, bound,
                            repeated(6, 10000, { task = "FIELD", kind = ACTION.HOLD }, "x>weigh", function(s, actor)
                                s.self.mp = actor.clock % 100
                            end)).split == true, "no split"
                    end),
                    case("watch: stuck escape fires", function()
                        local snap, bound = state("poet65")
                        return dogged(snap, bound,
                                repeated(8, 400, { task = "ESCAPE", kind = ACTION.MOVE }, "x>IMMINENT")).escape_stuck ==
                            true,
                            "no alarm"
                    end),
                    case("watch: no kills fires", function()
                        local snap, bound = state("poet65")
                        return dogged(snap, bound,
                            repeated(4, 100000, { task = "FIELD", kind = ACTION.HOLD }, "x>strike", function(_, actor)
                                actor.me.stats.kills = 0
                            end)).no_kills == true, "no alarm"
                    end),
                    case("watch: frail fires", function()
                        local snap, bound = state("poet65", { hp = 100 })
                        surround(snap, "양철인형", 2)
                        return dogged(snap, bound,
                            repeated(14, 10000, { task = "FIELD", kind = ACTION.HOLD }, "x>weigh", function(s, actor)
                                s.self.mp = actor.clock % 100
                            end)).frail == true, "no alarm"
                    end),
                    case("watch: errand in group fires", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        return
                            dogged(snap, bound,
                                { { at = 1000, decision = { task = "OUTFIT", kind = ACTION.HOLD }, path = "x>buy_gear" } })
                            .errand_in_group == true, "no alarm"
                    end),
                    case("watch: early supply fires on a profile", function()
                        local snap, bound = state("poet65")
                        snap.record.memory.untaught = {}
                        return
                            dogged(snap, bound,
                                { { at = 1000, decision = { task = "SUPPLY", kind = ACTION.MOVE }, path = "x>to_supply" } })
                            .supply_early == true, "no alarm"
                    end),
                    case("watch: ground flips fire", function()
                        local snap, bound = state("poet65")
                        local names = { "a", "b", "c", "d", "e" }
                        local index = 0
                        return dogged(snap, bound,
                            repeated(5, 10000, { task = "FIELD", kind = ACTION.HOLD }, "x>weigh", function(s)
                                index = index + 1
                                s.record.memory.solo = names[index]
                            end)).ground_flip == true, "no alarm"
                    end),
                    case("watch: a flight after an unready entry fires", function()
                        local snap = recorded("rogue_no_food")
                        local _, bound = recorded("rogue_no_food")
                        local roster = require("brain.policies.ground").roster_of(snap.world.map)
                        snap.record.memory.entry = { roster = roster, ready = false, at = 1 }
                        snap.record.memory.fled = {}
                        return
                            dogged(snap, bound,
                                { { at = 1000, decision = { task = "ESCAPE", kind = ACTION.MOVE }, path = "x>DOOMED" } })
                            .unready_flight == true, "no alarm"
                    end),
                    case("watch: a flight from a ground already given up fires", function()
                        local snap, bound = recorded("rogue_no_food")
                        local roster = require("brain.policies.ground").roster_of(snap.world.map)
                        snap.record.memory.entry = { roster = roster, ready = true, at = 1 }
                        snap.record.memory.fled = { ["r:" .. roster] = { power = require("brain.policies.danger").power(snap), strikes = 2, at = 0, map = snap.world.map } }
                        return
                            dogged(snap, bound,
                                { { at = 1000, decision = { task = "ESCAPE", kind = ACTION.MOVE }, path = "x>DOOMED" } })
                            .repeat_flight == true, "no alarm"
                    end),
                    case("watch: a first flight after a ready entry stays quiet", function()
                        local snap, bound = recorded("rogue_no_food")
                        local roster = require("brain.policies.ground").roster_of(snap.world.map)
                        snap.record.memory.entry = { roster = roster, ready = true, at = 1 }
                        snap.record.memory.fled = {}
                        local hits = dogged(snap, bound,
                            { { at = 1000, decision = { task = "ESCAPE", kind = ACTION.MOVE }, path = "x>DOOMED" } })
                        return hits.unready_flight == nil and hits.repeat_flight == nil, "fired"
                    end),
                    case("watch: waiting counts after a root ends", function()
                        local snap, bound       = state("poet65")
                        local fired             = {}
                        local dog               = require("sim.world.watch")
                        local actor             = {
                            clock = 0,
                            me = { stats = { kills = 1, exp = 0 } },
                            session = { actors = {} },
                            rooted = { from = 0, to = 10000 },
                            violate = function(_, rule) fired[rule] = true end
                        }
                        actor.session.actors[1] = actor
                        for index = 1, 40 do
                            actor.clock = index * 10000
                            snap.self.mp = index
                            dog.check(actor, snap, bound, { task = "FIELD", kind = ACTION.HOLD }, "x>wait_leader")
                        end
                        return fired["waiting:wait_leader"] == true, "no alarm"
                    end),
                    case("watch: bare rogue swings fire", function()
                        local snap, bound = state("rogue50")
                        surround(snap, "해골", 1)
                        return
                            dogged(snap, bound,
                                repeated(6, 500, { task = "DEFEND", kind = ACTION.ATTACK, key = "SWING" }, "x>weigh"))
                            .swing_with_spell == true, "no alarm"
                    end),
                    case("watch: cloaked rogue swings are quiet", function()
                        local snap, bound = state("rogue50")
                        snap.self.cloaked = true
                        surround(snap, "해골", 1)
                        return
                            dogged(snap, bound,
                                repeated(6, 500, { task = "DEFEND", kind = ACTION.ATTACK, key = "SWING" }, "x>weigh"))
                            .swing_with_spell ~= true, "fired"
                    end),
                    case("watch: quiet play fires nothing", function()
                        local snap, bound = state("poet65")
                        local fired = dogged(snap, bound,
                            repeated(10, 5000, { task = "FIELD", kind = ACTION.MOVE }, "x>strike", function(s, actor)
                                s.world.x = 10 + actor.clock % 7
                            end))
                        local names = {}
                        for rule in pairs(fired) do
                            table.insert(names, rule)
                        end
                        return #names == 0, table.concat(names, ",")
                    end),
                    case("safe fighter does not spend its mana on tiny self heals", function()
                        local snap, bound = recorded("fighter_tiny_self_heal")
                        snap.self.mp = 57
                        local pick = require("brain.tasks.aid").urgent(snap, bound)
                        return pick == nil, tostring(pick and pick.key)
                    end),
                    case("bot already out of the ground ignores the crew escape notice", function()
                        local snap, bound = recorded("notice_after_leaving")
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick == nil or pick.task ~= "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("standing still long after the last hit is not stuck", function()
                        local snap = state("poet65")
                        snap.self.struck = 600000
                        snap.record.memory.spot = { map = snap.world.map, x = snap.world.x, y = snap.world.y, at = 0 }
                        local idle = party.stalled(snap, data.swing)
                        snap.self.struck = 100
                        local hit = party.stalled(snap, data.swing)
                        return idle == false and hit == true, "idle=" .. tostring(idle) .. " hit=" .. tostring(hit)
                    end),
                    case("crew escape near the gate still uses the cardinal leap", function()
                        local snap, bound = recorded("crew_escape_walks")
                        local go = require("brain.trees.go")
                        local dest = require("brain.tasks.escape").flee(snap, bound, { name = "x" }, "CREW").dest
                        local chosen = go.choose(snap, bound, dest,
                            { closed = {}, perilous = {}, shunned = {}, spent = {} })
                        return chosen ~= nil and chosen.name == "비영사천문", tostring(chosen and chosen.name)
                    end),
                    case("hurt bot with mana heals before walking to the ground", function()
                        local snap, bound = recorded("hurt_walks_to_ground")
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task ~= "FIELD",
                            tostring(pick and pick.task) ..
                            " low=" .. tostring(require("brain.tasks.rest").low(snap, bound))
                            ..
                            " reach=" ..
                            math.floor(battle.reachable(snap)) ..
                            " peak=" ..
                            tostring(party.plan(snap, bound).peak) .. " need=" .. math.floor(ground.need(snap, bound))
                            .. " bneed=" ..
                            math.floor(battle.need(snap, bound)) .. " hp=" .. snap.self.hp .. "/" .. snap.self.max_hp
                    end),
                    case("low hp bot does not drink mana first", function()
                        local snap, bound = recorded("tiny_heal_over_food")
                        snap.self.mp = 407
                        snap.self.hp = 100
                        local pick = decide.choose(snap, bound, nil, nil)
                        local foods = {}
                        for _, dose in ipairs(require("brain.tasks.rest").edible(snap, bound)) do
                            table.insert(foods, dose.name .. ":" .. dose.amount)
                        end
                        local food = pick ~= nil and
                            (pick.kind ~= ACTION.USE or (items[pick.item] ~= nil and recovery.restored(snap, items[pick.item], "hp") > 0))
                        return food == true,
                            tostring(pick and pick.task) ..
                            " " .. tostring(pick and pick.kind) .. " " .. tostring(pick and pick.item)
                            .. " edible=[" ..
                            table.concat(foods, ",") .. "] hp=" .. snap.self.hp .. "/" .. snap.self.max_hp
                    end),
                    case("cardinal leap even one step from the gate", function()
                        local snap, bound = recorded("leap_one_step_from_gate")
                        local go = require("brain.trees.go")
                        local dest = require("brain.tasks.escape").flee(snap, bound, { name = "x" }, "CREW").dest
                        local chosen = go.choose(snap, bound, dest,
                            { closed = {}, perilous = {}, shunned = {}, spent = {} })
                        return chosen ~= nil and chosen.name == "비영사천문", tostring(chosen and chosen.name)
                    end),
                    case("no self-spending strike while the healer is out of mana", function()
                        local snap, bound = recorded("fighter_low_keeps_attacking")
                        snap.self.hp = 1853
                        snap.party[1].self.mp = 232
                        local picked = {}
                        for _, mob in ipairs(snap.world.mobs) do
                            if math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 and mob.hp ~= nil then
                                local choice = battle.choose(snap, bound, mob, true)
                                if choice ~= nil and choice.key == "건곤대나이" then
                                    table.insert(picked, mob.name)
                                end
                            end
                        end
                        return #picked == 0, table.concat(picked, ",")
                    end),
                    case("dying beside a healer out of mana escapes", function()
                        local snap, bound = recorded("fighter_low_keeps_attacking")
                        snap.self.hp = 294
                        snap.party[1].self.mp = 232
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("self-spending strike stays while the healer has mana", function()
                        local snap, bound = recorded("fighter_low_keeps_attacking")
                        snap.self.hp = 5557
                        snap.party[1].self.mp = 3142
                        local mob = require("brain.tasks.escape").adjacent_hitter(snap)
                        local choice = battle.choose(snap, bound, mob, true)
                        return choice ~= nil and choice.key == "건곤대나이", tostring(choice and choice.key)
                    end),
                    case("healer spends the charge reserve on a dying ally", function()
                        local snap = recorded("fighter_dying_with_ready_healer")
                        snap.self.hp = 427
                        local view, query = as_healer(snap)
                        view.self.casted = nil
                        local pick = decide.choose(view, query, nil, nil)
                        return pick ~= nil and pick.task == "AID", tostring(pick and pick.task)
                    end),
                    case("healer keeps the charge reserve for a healthy ally", function()
                        local snap = recorded("fighter_dying_with_ready_healer")
                        snap.self.hp = 6000
                        local view, query = as_healer(snap)
                        view.self.casted = nil
                        local pick = decide.choose(view, query, nil, nil)
                        return pick == nil or pick.task ~= "AID", tostring(pick and pick.task)
                    end),
                    case("follower joins the target its leader shoots from afar", function()
                        local snap, bound = recorded("healer_waits_beside_leader")
                        snap.bag = driver.stocked(snap.bag.capacity)
                        data.trace = true
                        decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        return string.find(path, "strike", 1, true) ~= nil, path
                    end),
                    case("hurt leader stays by its healer instead of walking on", function()
                        local snap, bound = recorded("leader_walks_from_healer")
                        snap.self.hp = 150
                        if require("brain.tasks.rest").low(snap, bound) == false then
                            return false, "premise: not low"
                        end
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "REST", tostring(pick and pick.task)
                    end),
                    case("hurt leader with no healer in sight does not wait for one", function()
                        local snap, bound = recorded("leader_walks_from_healer")
                        snap.self.hp = 150
                        snap.party[1].world.x = snap.world.x + 40
                        data.trace = true
                        decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        return path ~= "" and string.find(path, "await_heal", 1, true) == nil, path
                    end),
                    case("doomed flight leaves the ground instead of walking to the crew", function()
                        local snap, bound = recorded("rooted_healer_dies")
                        snap.self.hp = 727
                        local escape = require("brain.tasks.escape")
                        local mob = escape.adjacent_hitter(snap)
                        local dest = escape.flee(snap, bound, mob, "DOOMED").dest
                        return dest.maps ~= nil and dest.urgent == true and dest.x == nil,
                            tostring(dest.map) .. " " .. tostring(dest.x) .. " urgent=" .. tostring(dest.urgent)
                    end),
                    case("swinging is not flagged when the swing outhits every spell", function()
                        local snap, bound = recorded("weak_swing_over_spell")
                        local hits = swings_watched(snap, bound)
                        return hits.swing_with_spell == nil, "flagged"
                    end),
                    case("swinging is flagged when a spell outhits the swing", function()
                        local snap, bound = recorded("weak_swing_over_spell")
                        snap.self.dam = 0
                        snap.self.str = 0
                        snap.spells = driver.versed(snap.spells)
                        table.insert(snap.spells, { name = "백열참주", slot = 90 })
                        local hits = swings_watched(snap, bound)
                        return hits.swing_with_spell ~= nil, "not flagged"
                    end),
                    case("long approach to a mob stops to decide again on the way", function()
                        local snap = recorded("fighter_healer_walking_away")
                        snap.self.hp = snap.self.max_hp
                        snap.party[1].self.hp = snap.party[1].self.max_hp
                        snap.party[1].self.mp = snap.party[1].self.max_mp
                        snap.self.struck = nil
                        local mob = snap.world.mobs[1]
                        mob.x, mob.y = snap.world.x, snap.world.y - 1
                        snap.world.mobs = { mob }
                        local view, query = as_healer(snap)
                        local kept = {}
                        for _, learned in ipairs(view.spells) do
                            local spell = require("brain.data.spells").spells[learned.name]
                            if spell == nil or (spell.role ~= "DEBUFF" and spell.role ~= "BUFF") then
                                table.insert(kept, learned)
                            end
                        end
                        view.spells = kept
                        data.trace = true
                        local pick = decide.choose(view, query, nil, nil)
                        local rows = decide.offers[view.self.name] or {}
                        local path = tostring(rows[1] and rows[1].path)
                        local detail = path ..
                            " key=" .. tostring(pick and pick.key) .. " kind=" .. tostring(pick and pick.kind)
                        return pick ~= nil and pick.kind == ACTION.MOVE and pick.dest.range > 1, detail
                    end),
                    case("no self-spending strike that leaves too little for the next volley", function()
                        local snap, bound = recorded("fighter_spends_into_volley")
                        snap.self.hp = snap.self.max_hp
                        data.trace = true
                        local best = {}
                        for _, mob in ipairs(snap.world.mobs) do
                            if math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                                local choice = battle.choose(snap, bound, mob, true)
                                for _, rival in ipairs(choice and choice.rivals or {}) do
                                    if rival.key == "건곤대나이" and rival.value > (choice.value or 0) then
                                        table.insert(best, mob.name .. "=" .. math.floor(rival.value))
                                    end
                                end
                            end
                        end
                        return #best == 0, table.concat(best, ",")
                    end),
                    case("member whose walk failed and never moved since counts as stuck", function()
                        local snap = recorded("rooted_member_left_behind")
                        return party.stalled(snap, data.swing), "not stalled"
                    end),
                    case("member that moved after its failed walk is not stuck", function()
                        local snap = recorded("rooted_member_left_behind")
                        snap.record.memory.spot.at = snap.record.memory.blocked.at + 1
                        return party.stalled(snap, snap.now - snap.record.memory.spot.at + 1) == false, "stalled"
                    end),
                    case("repeated blocked walks retry within the rerank time", function()
                        local snap = state("poet65")
                        snap.record.failures.FIELD = {
                            failures = 8,
                            duration = 2250,
                            age = 0,
                            level = snap.self.level,
                            reason =
                            "BLOCKED",
                            map = snap.world.map
                        }
                        local left = require("brain.policies.failure").remaining(snap, "FIELD")
                        return left <= data.rerank, tostring(left)
                    end),
                    case("repeated other failures keep growing their backoff", function()
                        local snap = state("poet65")
                        snap.record.failures.SUPPLY = {
                            failures = 8,
                            duration = 2250,
                            age = 0,
                            level = snap.self.level,
                            reason =
                            "NO_ROUTE",
                            map = snap.world.map
                        }
                        local left = require("brain.policies.failure").remaining(snap, "SUPPLY")
                        return left == 2250 * 2 ^ 7, tostring(left)
                    end),
                    case("member hit while fighting still goes to its pinned crew", function()
                        local snap, bound = recorded("poet_fights_apart")
                        local pinned, lead = nil, nil
                        for _, member in ipairs(snap.party) do
                            pinned = member.pinned and member or pinned
                            lead = member.self.name == snap.leader and member or lead
                        end
                        if pinned == nil or lead == nil then
                            return false, "premise: no pinned crew or leader"
                        end
                        lead.rally = {
                            name = pinned.self.name,
                            map = pinned.world.map,
                            x = pinned.world.x,
                            y = pinned
                                .world.y
                        }
                        data.trace = true
                        local decision = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        local dest = decision ~= nil and decision.dest or {}
                        return string.find(path, "rally", 1, true) ~= nil and
                            (dest.map == pinned.world.map or (dest.maps or {})[1] == pinned.world.map), path
                    end),
                    case("a solo fighter that heals itself and can buy mana needs no hp supplies", function()
                        local snap, bound = recorded("rogue_no_food")
                        local need = require("brain.tasks.supply").need_of(snap, bound, "hp")
                        return need == nil, tostring(need and need.usage)
                    end),
                    case("a solo fighter without a heal skill still plans hp supplies", function()
                        local snap, bound = state("warrior63")
                        snap.spells = {}
                        if require("brain.policies.recovery").heal(snap) ~= nil then
                            return false, "premise: has a heal skill"
                        end
                        local need = require("brain.tasks.supply").need_of(snap, bound, "hp")
                        return need ~= nil and need.usage > 0, "no need"
                    end),
                    case("grouped fighter without food and a healer crewmate needs no hp supplies", function()
                        local snap, bound = recorded("warrior_no_food")
                        local need = require("brain.tasks.supply").need_of(snap, bound, "hp")
                        return need == nil, tostring(need and need.usage)
                    end),
                    case("no plan on the ground does not crash the group check", function()
                        local snap, bound = recorded("poet_fights_apart")
                        local real = party.plan
                        party.plan = function()
                            return nil
                        end
                        local ok, result = pcall(require("brain.trees.together").on_ground, snap, bound)
                        party.plan = real
                        return ok and result == false, tostring(result)
                    end),
                    case("follower ahead of its leader walks on instead of blocking the way", function()
                        local snap, bound = recorded("ally_blocks_corridor")
                        data.trace = true
                        local pick = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        local walking = string.find(path, "along", 1, true) ~= nil or
                            string.find(path, "make_way", 1, true) ~= nil
                        return walking and pick.kind == ACTION.MOVE, path
                    end),
                    case("crew escape and repeated flight leave the ground like a doomed flight", function()
                        local snap, bound = recorded("rogue_flees_on_foot")
                        snap.self.hp = 858
                        local escape = require("brain.tasks.escape")
                        local mob = escape.adjacent_hitter(snap)
                        local crew = escape.flee(snap, bound, mob, "CREW").dest
                        local again = escape.flee(snap, bound, mob, "FLEEING").dest
                        return crew.urgent == true and again.urgent == true and crew.x == nil and
                            crew.maps[1] ~= party.plan(snap, bound).gate, tostring(crew.maps and crew.maps[1])
                    end),
                    case("healer in more danger than its ally saves itself first", function()
                        local snap = recorded("fighter_dying_with_ready_healer")
                        snap.self.hp = 700
                        local view, query = as_healer(snap)
                        view.self.hp = 300
                        view.world.mobs = {}
                        for index, spot in ipairs({ { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }) do
                            local mob = snap.world.mobs[index]
                            mob.x, mob.y = view.world.x + spot[1], view.world.y + spot[2]
                            table.insert(view.world.mobs, mob)
                        end
                        data.trace = true
                        local pick = decide.choose(view, query, nil, nil)
                        local rows = decide.offers[view.self.name] or {}
                        local path = tostring(rows[1] and rows[1].path)
                        return pick ~= nil and (pick.oid == nil or pick.oid ~= 9001),
                            path .. " oid=" .. tostring(pick and pick.oid)
                    end),
                    case("without regeneration a crewmate's heal is counted on only under blows", function()
                        local rest = require("brain.tasks.rest")
                        local snap, bound = recorded("leader_walks_from_healer")
                        snap.party[1].self.mp = 2000
                        snap.self.hp = snap.self.max_hp // 2
                        snap.world.mobs = {}
                        local calm = rest.mending(snap, bound)
                        surround(snap, "양철인형", 1).attack = "AGGRESSIVE"
                        snap.self.struck = 0
                        local hit = rest.mending(snap, bound)
                        return calm == 0 and hit > 0, string.format("calm=%s hit=%s", calm, hit)
                    end),
                    case("without regeneration a bot one hp short with no recovery worth it still hunts", function()
                        local field       = require("brain.tasks.field")
                        local restore     = require("brain.trees.restore")
                        local snap, bound = state("warrior63")
                        local kept        = {}
                        for _, learned in ipairs(snap.spells) do
                            if (spells.spells[learned.name] or {}).role ~= "HEAL" then
                                table.insert(kept, learned)
                            end
                        end
                        snap.spells    = kept
                        snap.bag.items = {}
                        snap.self.hp   = snap.self.max_hp - 1
                        local plan     = party.plan(snap, bound)
                        local mendable = restore.mendable(snap, bound)
                        local wanted   = field.wanted(snap, bound)
                        return mendable == false and wanted == true,
                            string.format("mendable=%s wanted=%s hp=%s peak=%s", tostring(mendable), tostring(wanted),
                                snap.self.hp, tostring(plan and plan.peak))
                    end),
                    case("healer with mana heals a low ally that waits beside it", function()
                        local snap = recorded("leader_walks_from_healer")
                        snap.party[1].self.mp = 2000
                        snap.party[1].self.hp = snap.party[1].self.max_hp
                        local view, query = as_healer(snap)
                        local pick = decide.choose(view, query, nil, nil)
                        return pick ~= nil and pick.task == "AID" and pick.oid == 9001, tostring(pick and pick.task)
                    end),
                    case("leader stuck with a stuck member lets the member come", function()
                        local snap, bound = recorded("poet_fights_apart")
                        snap.leader = snap.self.name
                        snap.party[1].pinned = true
                        snap.record.memory.blocked = { at = snap.now, since = snap.now, map = snap.world.map }
                        data.trace = true
                        decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        return string.find(path, "left_behind", 1, true) == nil, path
                    end),
                    case("tiny self heal gives a fighter no reason to drink", function()
                        local snap, bound = recorded("warrior_dry_with_doses")
                        local hits = swings_watched_dry(snap, bound)
                        return hits.dry_with_doses == nil, "flagged"
                    end),
                    case("member standing in the doorway makes way for its leader", function()
                        local snap, bound = recorded("member_blocks_door")
                        data.trace = true
                        local pick = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        return string.find(path, "make_way", 1, true) ~= nil and pick.kind == ACTION.MOVE, path
                    end),
                    case("fighter whose healer died escapes instead of fighting on", function()
                        local snap, bound = recorded("alone_fighter_dies")
                        snap.party[1].self.ghost = true
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE", tostring(pick and pick.task)
                    end),
                    case("dead member shows as a ghost to its crew", function()
                        local list, brain_ = shared()
                        local warrior = suite.profile(list, "warrior63", 100000, brain_)
                        local poet = suite.profile(list, "warrior63", 100000, brain_)
                        poet.bag = { items = {}, used = 0, capacity = poet.bag.capacity }
                        local settings = setmetatable(
                            {
                                seed = 1,
                                output = NULL,
                                duration = 20000,
                                arrange = function(_, here)
                                    return { here, here }
                                end
                            }, { __index = roster.team })
                        local _, session = driver.team(brain_, {
                            { profile = warrior, overrides = { grouping = true } },
                            { profile = poet, overrides = { grouping = true, hp = 1, mp = 0 }, rooted = { from = 0, to = 20000 } },
                        }, settings, {})
                        local dead = session.actors[2]
                        local shown = session.profiles[dead]
                        return dead.me.dead == true and shown ~= nil and shown.self.ghost == true,
                            "dead=" .. tostring(dead.me.dead) .. " ghost=" .. tostring(shown and shown.self.ghost)
                    end),
                    case("grouped fighter out of food with a healer crewmate does not go for food", function()
                        local snap, bound = recorded("alone_fighter_dies")
                        snap.self.hp = snap.self.max_hp
                        snap.world.mobs = {}
                        snap.self.struck = nil
                        local supply = require("brain.tasks.supply")
                        local choice, _, _, _, _, _, _, _, kind = supply.order(snap, bound)
                        return kind ~= "hp", tostring(choice and choice.name) .. " " .. tostring(kind)
                    end),
                    case("surrounded caster low on both refills mana by charging, not by potion", function()
                        local snap, bound = recorded("charge_under_attack")
                        snap.self.hp, snap.self.mp = 1893, 204
                        data.trace = true
                        local wrong = {}
                        for _, mob in ipairs(snap.world.mobs) do
                            if math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                                local choice = battle.choose(snap, bound, mob, true)
                                local charge, potion = -math.huge, -math.huge
                                for _, rival in ipairs(choice and choice.rivals or {}) do
                                    if rival.key == "CHARGE" then
                                        charge = rival.value
                                    elseif rival.key == "동동주" or rival.key == "홍주" then
                                        potion = math.max(potion, rival.value)
                                    end
                                end
                                if charge == -math.huge or potion > charge then
                                    table.insert(wrong, mob.name .. ":" .. tostring(choice and choice.key))
                                end
                            end
                        end
                        return #wrong == 0, table.concat(wrong, ",")
                    end),
                    case("finishing a lap decides again at once", function()
                        local snap, bound = recorded("idle_after_lap")
                        snap.record.memory.servicing = nil
                        snap.record.memory.serviced = 0
                        snap.record.memory.search = { map = snap.world.map, from = 0 }
                        local plan = require("brain.policies.ground").choose(snap, bound)
                        snap.record.memory.solo = plan.name
                        snap.record.memory.commit = plan.name
                        snap.world.map = plan.floors[#plan.floors].map
                        snap.world.mobs = {}
                        snap.record.memory.search = { map = snap.world.map, from = 0 }
                        local left = {}
                        for _, floor in ipairs(plan.floors) do
                            left[floor.map] = snap.now
                        end
                        snap.record.memory.leg = { name = plan.name, map = snap.world.map, left = left }
                        data.trace = true
                        local pick = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        if string.find(path, "lap_done", 1, true) == nil then
                            return false, "premise: " .. path
                        end
                        return pick.revisit ~= nil and pick.revisit <= 1, tostring(pick.revisit)
                    end),
                    case("fighter with food in the bag does not flee a volley it can eat through", function()
                        local snap, bound = recorded("rogue_no_food")
                        snap.bag = driver.stocked(snap.bag.capacity)
                        local burst = battle.burst(snap)
                        if burst <= 0 or require("brain.tasks.escape").adjacent_hitter(snap) == nil then
                            return false, "premise: burst=" .. burst
                        end
                        snap.self.hp = math.floor(2 * burst - 250)
                        snap.self.struck = 0
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task ~= "ESCAPE",
                            tostring(pick and pick.task) .. " hp=" .. snap.self.hp .. " burst=" .. burst
                    end),
                    case("bot that fled a hunting ground skips grounds with the same mobs", function()
                        local snap, bound = recorded("rogue_no_food")
                        snap.record.memory.commit, snap.record.memory.solo, snap.record.memory.aim = nil, nil, nil
                        local g = require("brain.policies.ground")
                        local first = g.choose(snap, bound)
                        if first == nil then
                            return false, "premise: no ground"
                        end
                        local again, rebound = recorded("rogue_no_food")
                        again.record.memory.commit, again.record.memory.solo, again.record.memory.aim = nil, nil, nil
                        again.record.memory.fled = { ["r:" .. g.roster_of(first.floors[1].map)] = { power = require("brain.policies.danger").power(again), at = again.now, strikes = 2 } }
                        local second = g.choose(again, rebound)
                        local same = second ~= nil and
                            g.roster_of(second.floors[1].map) == g.roster_of(first.floors[1].map)
                        return second ~= nil and same == false,
                            tostring(first.name) .. " -> " .. tostring(second and second.name)
                    end),
                    case("fleeing a hunting ground releases only the commitment to it", function()
                        local snap = recorded("rogue_no_food")
                        local g = require("brain.policies.ground")
                        local roster = g.roster_of(snap.world.map)
                        if roster == nil then
                            return false, "premise: not on a ground " .. tostring(snap.world.map)
                        end
                        local here, other = nil, nil
                        for _, bundle in ipairs(hunting.bundles) do
                            local inside = false
                            for _, floor in ipairs(bundle.floors) do
                                inside = inside or floor.map == snap.world.map
                            end
                            if inside then
                                here = here or bundle.name
                            else
                                other = other or bundle.name
                            end
                        end
                        snap.record.memory.fled, snap.record.memory.commit, snap.record.memory.solo = {}, nil, here
                        local fled, released = decide.flee(snap, { task = "ESCAPE", revive = 0 })
                        snap.record.memory.fled, snap.record.memory.solo = {}, other
                        local _, kept = decide.flee(snap, { task = "ESCAPE", revive = 0 })
                        return fled ~= nil and released == true and kept == false and fled["r:" .. roster] ~= nil,
                            string.format("here=%s released=%s other=%s released=%s", tostring(here), tostring(released),
                                tostring(other), tostring(kept))
                    end),
                    case("a doomed flight also marks the ground's mobs as too much for now", function()
                        local snap = recorded("rogue_no_food")
                        local roster = require("brain.policies.ground").roster_of(snap.world.map)
                        snap.record.memory.fled = {}
                        local fled = decide.flee(snap, { task = "ESCAPE", revive = 0, proven = false })
                        return fled ~= nil and roster ~= nil and fled["r:" .. roster] ~= nil and
                            fled[tostring(snap.world.map)] == nil,
                            tostring(fled ~= nil)
                    end),
                    case("mage that fled its ground still finds another", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = state("mage60")
                        local first = g.choose(snap, bound)
                        local again, rebound = state("mage60")
                        again.record.memory.fled = { ["r:" .. g.roster_of(first.floors[1].map)] = { power = require("brain.policies.danger").power(again), at = 0, strikes = 2 } }
                        local second = g.choose(again, rebound)
                        return second ~= nil and second.rate == second.rate,
                            tostring(first.name) .. " -> " .. tostring(second and second.name)
                    end),
                    case("watch: a bot with nothing to do for long fires", function()
                        local snap, bound       = state("poet65")
                        local fired             = {}
                        local dog               = require("sim.world.watch")
                        local actor             = {
                            clock = 0,
                            me = { stats = { kills = 1, exp = 0 } },
                            session = { actors = {} },
                            violate = function(_, rule) fired[rule] = true end
                        }
                        actor.session.actors[1] = actor
                        for index = 1, 10 do
                            actor.clock = index * 10000
                            snap.self.hp = snap.self.max_hp - index
                            dog.check(actor, snap, bound, nil, "")
                        end
                        return fired.nothing == true, "no alarm"
                    end),
                    case("fighter below two volleys of its ground eats before moving on", function()
                        local snap, bound = recorded("rogue_out_of_supplies")
                        snap.world.mobs = {}
                        snap.self.struck = nil
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "REST",
                            tostring(pick and pick.task) .. " hp=" .. snap.self.hp
                    end),
                    case("a fresh flight is not hidden behind an older cached ranking", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("rogue_no_food")
                        snap.record.memory.commit, snap.record.memory.solo, snap.record.memory.aim = nil, nil, nil
                        local first = g.choose(snap, bound)
                        local again, rebound = recorded("rogue_no_food")
                        again.record.memory.commit, again.record.memory.solo, again.record.memory.aim = nil, nil, nil
                        again.record.memory.fled = { ["r:" .. g.roster_of(first.floors[1].map)] = { power = require("brain.policies.danger").power(again), at = again.now, strikes = 2 } }
                        local second = g.choose(again, rebound)
                        local same = second ~= nil and
                            g.roster_of(second.floors[1].map) == g.roster_of(first.floors[1].map)
                        return second ~= nil and same == false,
                            tostring(first.name) .. " -> " .. tostring(second and second.name)
                    end),
                    case("one flight after a ready entry does not shut the ground", function()
                        local g = require("brain.policies.ground")
                        local function fresh(fled)
                            local snap, bound = recorded("rogue_no_food")
                            snap.record.memory.commit, snap.record.memory.solo, snap.record.memory.aim = nil, nil, nil
                            snap.record.memory.fled = fled
                            return g.choose(snap, bound), snap
                        end
                        local plain, snap = fresh({})
                        local marked = fresh({ ["r:" .. g.roster_of(plain.floors[1].map)] = { power = require("brain.policies.danger").power(snap), at = 0, strikes = 1 } })
                        return marked ~= nil and g.roster_of(marked.floors[1].map) == g.roster_of(plain.floors[1].map),
                            tostring(plain.name) .. " -> " .. tostring(marked and marked.name)
                    end),
                    case("flights count only after entering ready", function()
                        local snap = recorded("rogue_no_food")
                        local roster = require("brain.policies.ground").roster_of(snap.world.map)
                        snap.record.memory.fled = {}
                        snap.record.memory.entry = { roster = roster, ready = false, at = 0 }
                        local unready = decide.flee(snap, { task = "ESCAPE", revive = 0, proven = false })
                        local after_unready = unready["r:" .. roster].strikes
                        snap.record.memory.entry = { roster = roster, ready = true, at = 0 }
                        decide.flee(snap, { task = "ESCAPE", revive = 0, proven = false })
                        local once = snap.record.memory.fled["r:" .. roster].strikes
                        decide.flee(snap, { task = "ESCAPE", revive = 0, proven = false })
                        local twice = snap.record.memory.fled["r:" .. roster].strikes
                        return after_unready == 0 and once == 1 and twice == 2,
                            after_unready .. "/" .. once .. "/" .. twice
                    end),
                    case("entering a ground notes whether the bot came ready", function()
                        local snap, bound = recorded("rogue_out_of_supplies")
                        snap.record.memory.entry = nil
                        snap.world.mobs = {}
                        local moved, low = decide.entered(snap, bound)
                        snap.record.memory.entry = nil
                        snap.self.hp, snap.self.mp = snap.self.max_hp, snap.self.max_mp
                        local _, full = decide.entered(snap, bound)
                        return moved and low ~= nil and low.ready == false and full ~= nil and full.ready == true,
                            tostring(low and low.ready) .. "/" .. tostring(full and full.ready)
                    end),
                    case("bot outside its ground fills hp and mp before going in", function()
                        local snap, bound = recorded("rogue_out_of_supplies")
                        snap.world.mobs = {}
                        snap.self.struck = nil
                        snap.world.map = 14
                        snap.self.hp = snap.self.max_hp - 100
                        local rest = require("brain.tasks.rest")
                        local pick = decide.choose(snap, bound, nil, nil)
                        return rest.outside(snap, bound) and rest.low(snap, bound) and pick ~= nil and
                            pick.task == "REST",
                            tostring(rest.outside(snap, bound)) .. " " .. tostring(pick and pick.task)
                    end),
                    case("charge that cannot survive the next volley gives way to a potion", function()
                        local snap, bound = recorded("charge_under_attack")
                        snap.self.hp, snap.self.mp = math.floor(battle.burst(snap) * 2.5), 204
                        local drinks = {}
                        for _, item in ipairs(driver.stocked(snap.bag.capacity).items) do
                            if item.name == "동동주" or item.name == "홍주" then
                                table.insert(drinks, item)
                            end
                        end
                        snap.bag = { items = drinks, used = #drinks, capacity = snap.bag.capacity }
                        data.trace = true
                        decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        local charged = {}
                        for _, mob in ipairs(snap.world.mobs) do
                            if math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
                                local choice = battle.choose(snap, bound, mob, true)
                                if choice ~= nil and choice.key == "CHARGE" then
                                    table.insert(charged, mob.name)
                                end
                            end
                        end
                        return string.find(path, "charge", 1, true) == nil and #charged == 0,
                            path ..
                            " " ..
                            table.concat(charged, ",") ..
                            " charged=" .. recovery.charged(snap) .. " burst=" .. battle.burst(snap)
                    end),
                    case("watch: waiting while a crewmate walks toward us is not idle", function()
                        local snap, bound       = apart_floors("warrior63", "poet65", { task = "FIELD" })
                        local fired             = {}
                        local dog               = require("sim.world.watch")
                        local actor             = {
                            clock = 0,
                            me = { stats = { kills = 1, exp = 0 } },
                            session = { actors = {} },
                            violate = function(_, rule) fired[rule] = true end
                        }
                        actor.session.actors[1] = actor
                        for index = 1, 10 do
                            actor.clock = index * 10000
                            local mate = setmetatable(
                                { world = { map = snap.party[1].world.map, x = index, y = index } },
                                { __index = snap.party[1] })
                            local moment = setmetatable({ party = { mate } }, { __index = snap })
                            dog.check(actor, moment, bound, { task = "FIELD", kind = ACTION.HOLD }, "x>lead_gate")
                        end
                        return fired.idle == nil, "idle fired"
                    end),
                    case("leader fighting on its own floor still goes back for a stuck member", function()
                        local snap, bound = apart_floors("warrior63", "poet65", { task = "FIELD", pinned = true })
                        snap.self.struck = 0
                        snap.record.memory.spot = { map = snap.world.map, x = snap.world.x, y = snap.world.y, at = 0 }
                        data.trace = true
                        local decision = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        local dest = decision ~= nil and decision.dest or {}
                        local member = snap.party[1]
                        return string.find(path, "rally", 1, true) ~= nil and
                            (dest.map == member.world.map or (dest.maps or {})[1] == member.world.map),
                            path .. " dest=" .. tostring(dest.map or (dest.maps or {})[1])
                    end),
                    case("grouped bot does not leave a crewmate who is fighting", function()
                        local snap, bound = recorded("alone_fighter_dies")
                        snap.self.hp = snap.self.max_hp
                        snap.world.mobs = {}
                        snap.self.struck = nil
                        snap.party[1].task = "DEFEND"
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick == nil or (pick.task ~= "SUPPLY" and pick.task ~= "OUTFIT" and pick.task ~= "SELL"),
                            tostring(pick and pick.task)
                    end),
                    case("ground fled twice also rules out grounds at least as dangerous", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("rogue_no_food")
                        snap.record.memory.commit, snap.record.memory.solo, snap.record.memory.aim = nil, nil, nil
                        local first = g.choose(snap, bound)
                        local again, rebound = recorded("rogue_no_food")
                        again.record.memory.commit, again.record.memory.solo, again.record.memory.aim = nil, nil, nil
                        again.record.memory.fled = {
                            ["r:" .. g.roster_of(first.floors[1].map)] = {
                                power = require("brain.policies.danger").power(again),
                                at = 0,
                                strikes = 2,
                                map = first
                                    .floors[1].map
                            }
                        }
                        local second = g.choose(again, rebound)
                        local function worst(plan)
                            local most = 0
                            for _, floor in ipairs(plan ~= nil and plan.floors or {}) do
                                most = math.max(most, floor.peak or 0)
                            end
                            return most
                        end
                        return second ~= nil and worst(second) < worst(first),
                            tostring(first.name) ..
                            ":" .. worst(first) .. " -> " .. tostring(second and second.name) .. ":" .. worst(second)
                    end),
                    case("fighter close to two volleys eats instead of fleeing when food is at hand", function()
                        local snap, bound = recorded("rogue_no_food")
                        snap.bag = driver.stocked(snap.bag.capacity)
                        local burst = battle.burst(snap)
                        if burst <= 0 then
                            return false, "premise: no attacker"
                        end
                        snap.self.hp = math.floor(1.3 * burst)
                        snap.self.struck = 0
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task ~= "ESCAPE",
                            tostring(pick and pick.task) .. " hp=" .. snap.self.hp .. " burst=" .. burst
                    end),
                    case("leader whose old walk failed still fetches a rooted member", function()
                        local snap, bound = recorded("leader_ignores_rooted_member")
                        snap.world.mobs = {}
                        snap.self.struck = nil
                        snap.party[1].heading = nil
                        data.trace = true
                        local decision = decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        local dest = decision ~= nil and decision.dest or {}
                        local member = snap.party[1]
                        return string.find(path, "rally", 1, true) ~= nil and
                            (dest.map == member.world.map or (dest.maps or {})[1] == member.world.map),
                            path .. " dest=" .. tostring(dest.map or (dest.maps or {})[1])
                    end),
                    case("moving to the next floor keeps how the bot came into the ground", function()
                        local snap, bound = recorded("rogue_out_of_supplies")
                        snap.record.memory.entry = { roster = "other", ready = true, at = 5 }
                        snap.self.hp, snap.self.mp = 1, 0
                        local moved, entry = decide.entered(snap, bound)
                        return moved and entry ~= nil and entry.ready == true and entry.at == 5,
                            tostring(entry and entry.ready)
                    end),
                    case("group avoids a ground a member gave up on", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("poet_fights_apart")
                        local first = g.choose(snap, bound)
                        if first == nil then
                            return false, "premise: no ground"
                        end
                        local again, rebound = recorded("poet_fights_apart")
                        local member = require("brain.policies.escort").member(again.party[1])
                        again.party[1].fled = {
                            ["r:" .. g.roster_of(first.floors[1].map)] = {
                                power = require("brain.policies.danger").power(member),
                                at = 0,
                                strikes = 2,
                                map = first
                                    .floors[1].map
                            }
                        }
                        local crew = g.crew({ require("brain.policies.escort").member(again.party[1]) }, rebound)
                        local second = g.choose(again, rebound, nil, crew)
                        return second ~= nil and g.roster_of(second.floors[1].map) ~= g.roster_of(first.floors[1].map),
                            tostring(first.name) .. " -> " .. tostring(second and second.name)
                    end),
                    case("bot walled into a pocket leaves by teleport", function()
                        local snap, bound = recorded("rogue_out_of_supplies")
                        snap.world.mobs = {}
                        snap.self.struck = nil
                        snap.self.hp, snap.self.mp = snap.self.max_hp, snap.self.max_mp
                        snap.record.memory.blocked = { at = snap.now, since = snap.now - 60000, map = snap.world.map, walls = true }
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "ESCAPE" and pick.dest ~= nil and pick.dest.urgent == true,
                            tostring(pick and pick.task)
                    end),
                    case("unreachable mob is skipped when a long range kill does not pay", function()
                        local snap, bound = state("warrior63")
                        table.insert(snap.world.mobs, mob_named("양철인형", snap.world.x + 3, snap.world.y, 201))
                        local found = battle.target(snap, walled(bound), any)
                        return found == nil, tostring(found and found.choice.key)
                    end),
                    case("ranged bot still attacks an unreachable mob when it pays", function()
                        local snap, bound = state("mage60")
                        local plan = ground.choose(snap, bound)
                        table.insert(snap.world.mobs,
                            mob_named(plan.floors[1].targets[1], snap.world.x + 3, snap.world.y, 202))
                        return battle.target(snap, walled(bound), any) ~= nil, "no target"
                    end),
                    case("profile drinks hold their full uses per slot", function()
                        local snap  = state("poet65")
                        local total = 0
                        for _, dose in ipairs(recovery.doses(snap, "mp")) do
                            if dose.name == "동동주" then
                                total = dose.count
                            end
                        end
                        return total >= 200 * 25, "count=" .. total
                    end),
                    case("a solo rogue spends no hp on a spell its ambush outpaces even with food in the bag", function()
                        local snap, bound = captured("sim1_91")
                        local result      = combat.outcome(snap, bound, mob_named("이쁘니인어", 11, 10, 77))
                        return (result.spent or 0) == 0 and recovery.refilling(snap, "hp") > 0,
                            string.format("spent=%s casts=%s", tostring(result.spent), tostring(result.casts))
                    end),
                    case("the fight model plans no spell paid with hp", function()
                        local options = {
                            offense = {
                                { name = "bled", damage = 120, hp = -120, mp = 0 },
                                { name = "paid", damage = 300, hp = 0, mp = 30 }
                            },
                            heal = {},
                            strike = {}
                        }
                        local kept    = combat.bloodless(options).offense
                        return #kept == 1 and kept[1].name == "paid", "kept=" .. #kept
                    end),
                    case("a run-away mob comes on sight and hits back until it flees", function()
                        local danger      = require("brain.policies.danger")
                        local snap, bound = captured("sim1_91")
                        local mob         = mob_named("이쁘니인어", 11, 10, 77)
                        local fleeing     = mob_named("이쁘니인어", 11, 10, 78)
                        fleeing.ratio     = 0.1
                        local result      = combat.outcome(snap, bound, mob)
                        local general     = mob_named("인어장군", 12, 10, 79)
                        return combat.hits(mob) and combat.seeks(mob) and result.taken > (result.spent or 0) and
                            math.abs(combat.striking(mob) - 0.8) < 1e-9 and combat.striking(fleeing) == 0 and
                            danger.hostile(snap, bound, general),
                            string.format("taken=%s spent=%s striking=%s fleeing=%s hostile=%s", tostring(result.taken),
                                tostring(result.spent), combat.striking(mob), combat.striking(fleeing),
                                tostring(danger.hostile(snap, bound, general)))
                    end),
                    function()
                        return scene("pair enters from the gate together", { "warrior63", "poet65" }, {
                            goal = "entered",
                            sparse = 0,
                        })
                    end,
                    function()
                        return scene("pair moves to the next floor with no prey left", { "rogue50", "poet65" }, {
                            goal = "advanced",
                            sparse = 0,
                            arrange = function(_, here)
                                return { here, here }
                            end,
                        })
                    end,
                    function()
                        return scene("trio moves to the next floor without bouncing",
                            { "warrior63", "rogue50", "poet65" }, {
                                goal = "advanced",
                                sparse = 0,
                                arrange = function(_, here)
                                    return { here, here, here }
                                end,
                            })
                    end,
                    function()
                        return scene("trio does not bounce while one member is rooted",
                            { "warrior63", "rogue50", "poet65" }, {
                                goal = "advanced",
                                sparse = 0,
                                rooted = { [2] = { from = 0, to = 45000 } },
                                arrange = function(_, here)
                                    return { here, here, here }
                                end,
                            })
                    end,
                    function()
                        local list, brain_ = shared()
                        local snap = suite.profile(list, "warrior63", 100000, brain_)
                        snap.bag = { items = {}, used = 0, capacity = snap.bag.capacity }
                        local alarm = os.tmpname()
                        local settings = setmetatable({
                            seed = 1,
                            output = NULL,
                            guard = true,
                            halt = true,
                            stall = 30000,
                            bounce = 1000,
                            alarm = alarm,
                            drops = false,
                            duration = 60000,
                            arrange = function(_, here)
                                return { here }
                            end,
                        }, { __index = roster.team })
                        local _, session = driver.team(brain_,
                            { { profile = snap, overrides = { hp = 1, mp = 0 }, rooted = { from = 0, to = 60000 } } },
                            settings, {})
                        local file = io.open(alarm, "r")
                        local text = file ~= nil and file:read("a") or ""
                        if file ~= nil then
                            file:close()
                        end
                        os.remove(alarm)
                        return report("death raises the alarm", string.find(text, "died", 1, true) ~= nil,
                            tostring(session.verdict) ..
                            " dead=" .. tostring(session.actors[1].me.dead) .. " t=" .. session.clock)
                    end,
                    function()
                        return scene("group ends up together when a member is stuck on another floor",
                            { "rogue50", "poet65" }, {
                                goal = "reunited",
                                seconds = 120,
                                rooted = { [2] = { from = 0, to = 30000 } },
                                arrange = function(planned, here)
                                    return { here, { map = planned.floors[2].map } }
                                end,
                            })
                    end,
                    function()
                        return scene("leader does not bounce through a door on the way to the gate",
                            { "warrior63", "poet65" }, {
                                goal = "entered",
                                sparse = 0,
                                ground = { bundle = "졸본해골굴" },
                                arrange = function()
                                    return { { map = 22, x = 1, y = 107 }, { map = 3180, x = 28, y = 14 } }
                                end,
                            })
                    end,
                    function()
                        return scene("team sent to a forced ground starts at that ground's gate",
                            { "warrior63", "poet65" }, {
                                goal = "entered",
                                sparse = 0,
                                seconds = 30,
                                ground = { bundle = "국내성흉가" },
                            })
                    end,
                    function()
                        return scene("crew with gear to buy still follows to the next floor", { "warrior63", "poet65" },
                            {
                                goal = "advanced",
                                sparse = 0,
                                arrange = function(_, here)
                                    return { here, here }
                                end,
                            })
                    end,
                    function()
                        return scene("leader in a shop joins crew already on the ground", { "poet65", "warrior63" }, {
                            goal = "entered",
                            sparse = 0,
                            arrange = function(_, here)
                                return { { map = 8179, x = 3, y = 5 }, here }
                            end,
                        })
                    end,
                    function()
                        return scene("crew with no route to each other still reach the ground", { "rogue50", "poet65" },
                            {
                                goal = "entered",
                                sparse = 0,
                                arrange = function()
                                    return { { map = 8153, x = 13, y = 5 }, { map = 8151, x = 12, y = 8 } }
                                end,
                            })
                    end,
                    function()
                        return scene("group forms and picks a ground", { "warrior63", "poet65" }, {
                            goal = "formed",
                            sparse = 0,
                            ground = { form = true, map = 22 },
                            seconds = 30,
                        })
                    end,
                    function()
                        return scene("group buys supplies before entering", { "warrior63", "poet65" }, {
                            goal = "supplied",
                            sparse = 0,
                            bare = true,
                            ground = { form = true, map = 22 },
                            seconds = 300,
                        })
                    end,
                },
            },
        },
    },
}
