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

local function buffed(snap)
    local active = {}
    for _, name in ipairs(snap.buffs or {}) do
        active[name] = true
    end
    for _, learned in ipairs(snap.spells or {}) do
        local spell = spells.spells[learned.name]
        if spell ~= nil and spell.role == "BUFF" then
            active[learned.name] = true
        end
    end
    local list = {}
    for name in pairs(active) do
        table.insert(list, name)
    end
    table.sort(list)
    return list
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

local function open_ground(bound, map)
    local function open(cx, cy)
        for x = cx - 1, cx + 3 do
            for y = cy - 2, cy + 2 do
                if bound.floor(map, x, y) == false then
                    return false
                end
            end
        end
        return true
    end
    for y = 3, 80 do
        for x = 3, 80 do
            if open(x, y) then
                return x, y
            end
        end
    end
    return nil, nil
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
                    case("a group keeps off a ground whose respawn another group already takes, not off one with respawn to spare", function()
                        local rerank = require("brain.data.combat").rerank
                        local function planned(claim, shift)
                            local snap, bound = state("warrior93")
                            snap.now = snap.now + shift * rerank
                            snap.world.map, snap.world.x, snap.world.y = 22, 16, 103
                            snap.party, snap.group = {}, {}
                            for _, name in ipairs({ "poet94", "mage95" }) do
                                local mate = state(name)
                                mate.now = snap.now
                                mate.world.map, mate.world.x, mate.world.y = 22, 16, 103
                                table.insert(snap.party, party.profile(mate, bound, nil))
                                table.insert(snap.group, mate.self.name)
                            end
                            snap.leader  = snap.self.name
                            bound.demand = function(map)
                                return map == 2376 and claim or 0
                            end
                            local plan   = party.plan(snap, bound)
                            local posted = party.profile(snap, bound, nil).plan
                            local first  = posted ~= nil and posted.floors[1] or {}
                            return plan ~= nil and plan.name or "nil", first.kills
                        end
                        local free, kills = planned(0, 40)
                        local spare       = planned(0.005, 50)
                        local taken       = planned(0.3, 60)
                        return free == "흑해골굴" and spare == "흑해골굴" and taken ~= "흑해골굴" and type(kills) == "number" and
                            kills > 0,
                            string.format("free=%s spare=%s taken=%s posted kills=%s", free, spare, taken,
                                tostring(kills))
                    end),
                    case("a bot leaves a ground somebody hunts for an empty one, and one far off for one near", function()
                        local ground        = require("brain.policies.ground")
                        local rerank        = require("brain.data.combat").rerank
                        local first, bound0 = state("warrior63")
                        local chosen        = ground.choose(first, bound0)
                        local held          = {}
                        for _, floor in ipairs(chosen ~= nil and chosen.floors or {}) do
                            held[floor.map] = true
                        end
                        local taken, bound1 = state("warrior63")
                        bound1.demand       = function(map)
                            return held[map] and 100 or 0
                        end
                        local other         = ground.choose(taken, bound1)
                        local far, bound2   = state("warrior63")
                        far.now             = far.now + 6 * rerank
                        local measured      = bound2.distances
                        bound2.distances    = function(detour, from)
                            local found = measured(detour, from) or {}
                            for map in pairs(held) do
                                found[map] = 100000
                            end
                            return found
                        end
                        local away          = ground.choose(far, bound2)
                        return chosen ~= nil and other ~= nil and away ~= nil and other.name ~= chosen.name and
                            away.name ~= chosen.name,
                            string.format("chosen=%s held=%s far=%s", tostring(chosen and chosen.name),
                                tostring(other and other.name), tostring(away and away.name))
                    end),
                    case("a bot's plan is kept over a plan made after it, and walked on under its own id", function()
                        local snap, bound = state("warrior93")
                        local me          = snap.self.name
                        local steps       = { { x = 30, y = 27 }, { x = 30, y = 28 } }
                        bound.direct(me,
                            { id = "early", map = 22, steps = steps, done = 1, made = 100, members = { me } })
                        bound.direct(me, { id = "late", map = 22, steps = steps, done = 1, made = 200, members = { me } })
                        local kept = (bound.directed(me) or {}).id
                        bound.direct(me,
                            { id = "early", map = 22, steps = steps, done = 2, made = 100, members = { me } })
                        local walked = (bound.directed(me) or {}).done
                        bound.direct(me, nil)
                        bound.direct(me, { id = "late", map = 22, steps = steps, done = 1, made = 200, members = { me } })
                        local fresh = (bound.directed(me) or {}).id
                        bound.direct(me, nil)
                        return kept == "early" and walked == 2 and fresh == "late",
                            string.format("kept=%s walked=%s fresh=%s", tostring(kept), tostring(walked), tostring(fresh))
                    end),
                    case("a walk stopped at a bot already moved by a plan waits for that plan instead of making another", function()
                        local traffic = require("brain.policies.traffic")
                        local function untangled(planned)
                            local snap, bound                          = state("warrior93")
                            local me                                   = snap.self.name
                            local other                                = "untangle-peer"
                            snap.world.map, snap.world.x, snap.world.y = 22, 30, 27
                            snap.world.mobs, snap.world.occupied       = {}, {}
                            bound.roster                               = function()
                                return { { name = other, profile = { self = { ghost = false }, world = { map = 22, x = 30, y = 28 } } } }
                            end
                            bound.ally                                 = function(name)
                                return name == other and { oid = 9002, x = 30, y = 28 } or nil
                            end
                            if planned then
                                bound.direct(other,
                                    {
                                        id = "first",
                                        map = 22,
                                        steps = { { x = 30, y = 28 }, { x = 31, y = 28 } },
                                        done = 1,
                                        made = 1,
                                        members = { other }
                                    })
                            end
                            local made   = traffic.untangle(nil, nil, snap, bound, { range = 0 },
                                { x = 30, y = 28, gx = 30, gy = 29 })
                            local theirs = (bound.directed(other) or {}).id
                            local mine   = bound.directed(me)
                            bound.direct(other, nil)
                            bound.direct(me, nil)
                            return made, theirs, mine ~= nil
                        end
                        local free, freed, joined  = untangled(false)
                        local held, kept, intruded = untangled(true)
                        return free == true and joined and held == false and kept == "first" and intruded == false,
                            string.format("free=%s freed=%s joined=%s held=%s kept=%s intruded=%s", tostring(free),
                                tostring(freed), tostring(joined), tostring(held), tostring(kept), tostring(intruded))
                    end),
                    case("two bots heading for each other's tile on open ground get past each other in one plan", function()
                        local snap, bound = state("warrior93")
                        local paths       = bound.untangle(2377, {
                            { x = 7, y = 3, gx = 6, gy = 3, range = 0, priority = 2 },
                            { x = 6, y = 3, gx = 7, gy = 3, range = 0, priority = 1 },
                        }, {}, 64)
                        local lead        = paths ~= nil and paths[1][#paths[1]] or {}
                        local poet        = paths ~= nil and paths[2][#paths[2]] or {}
                        return lead.x == 6 and lead.y == 3 and poet.x == 7 and poet.y == 3,
                            string.format("plan=%s lead=%s,%s poet=%s,%s", tostring(paths ~= nil), tostring(lead.x),
                                tostring(lead.y), tostring(poet.x), tostring(poet.y))
                    end),
                    case("a walk counted to a mob does not cross a warp it is not going to", function()
                        local snap, bound = state("warrior93")
                        local beyond      = bound.steps(10071, 9, 14, { { x = 12, y = 11, range = 1 } })[1]
                        local onto        = bound.steps(10071, 9, 14, { { x = 13, y = 15, range = 0 } })[1]
                        return beyond == false and type(onto) == "number",
                            string.format("beyond=%s onto=%s", tostring(beyond), tostring(onto))
                    end),
                    case("a mob a walk found no way to is not walked at again while it stays on its tile", function()
                        local battle                               = require("brain.policies.battle")
                        local snap, bound                          = state("warrior93")
                        snap.world.map, snap.world.x, snap.world.y = 10071, 9, 14
                        local mob                                  = mob_named("도깨비", 12, 11, 901)
                        local visible                              = { ["901"] = mob }
                        snap.record.memory.unreachable             = battle.unreached(nil, 901, 10071, snap.now, visible)
                        local kept                                 = battle.blocked(snap, mob) ~= nil
                        local moved                                = mob_named("도깨비", 12, 12, 901)
                        local loose                                = battle.blocked(snap, moved) == nil
                        local left                                 = battle.unreached(snap.record.memory.unreachable, 902,
                            10071, snap.now,
                            { ["901"] = moved })
                        return kept and loose and left == nil,
                            string.format("kept=%s loose=%s left=%s", tostring(kept), tostring(loose), tostring(left))
                    end),
                    case("a member does not wait beside a crewmate across the two-wide way to a door", function()
                        local together = require("brain.trees.together")
                        local door     = { x = 30, y = 30 }
                        local function waited(beside)
                            local snap, bound = state("mage95")
                            snap.world.map, snap.world.x, snap.world.y = 22, 31, 28
                            snap.world.mobs, snap.world.occupied = {}, {}
                            if beside then
                                local mate = state("rogue92")
                                mate.world.map, mate.world.x, mate.world.y = 22, 30, 28
                                snap.party = { party.profile(mate, bound, nil) }
                                snap.group = { mate.self.name, snap.self.name }
                            end
                            return together.waiting(snap, bound, door), bound
                        end
                        local alone        = waited(false)
                        local moved, bound = waited(true)
                        local dest         = moved ~= nil and moved.dest or {}
                        local apart        = dest.x ~= nil and not (dest.y == 28 and (dest.x == 30 or dest.x == 31))
                        return alone == nil and apart and bound.lane(22, dest.x, dest.y) == false,
                            string.format("alone=%s dest=%s,%s", tostring(alone), tostring(dest.x), tostring(dest.y))
                    end),
                    case("a bot that stepped aside keeps its tile while another bot of the plan walks", function()
                        local traffic                              = require("brain.policies.traffic")
                        local snap, bound                          = state("poet94")
                        snap.world.map, snap.world.x, snap.world.y = 22, 31, 27
                        local me                                   = snap.self.name
                        local other                                = "untangle-peer"
                        bound.ally                                 = function(name)
                            return name == other and { oid = 9002, x = 30, y = 28 } or nil
                        end
                        bound.direct(me,
                            {
                                id = "p",
                                map = 22,
                                steps = { { x = 31, y = 28 }, { x = 31, y = 27 } },
                                done = 1,
                                members = { me, other }
                            })
                        bound.direct(other, {
                            id = "p",
                            map = 22,
                            steps = { { x = 30, y = 28 }, { x = 31, y = 28 } },
                            done = 1,
                            members = { me, other }
                        })
                        local keeps = traffic.follow(snap, bound)
                        bound.direct(other, nil)
                        local ends = traffic.follow(snap, bound)
                        local left = bound.directed(me)
                        bound.direct(me, nil)
                        return keeps ~= nil and keeps.kind == ACTION.HOLD and ends == nil and left == nil,
                            string.format("keeps=%s ends=%s left=%s", tostring(keeps and keeps.kind), tostring(ends),
                                tostring(left))
                    end),
                    case("a part that ends on a warp ends one step before it, for the bot's own walk to take it in", function()
                        local traffic                              = require("brain.policies.traffic")
                        local snap, bound                          = state("warrior93")
                        local me                                   = snap.self.name
                        local steps                                = { { x = 30, y = 27 }, { x = 30, y = 28 }, { x = 30, y = 29 }, { x = 30, y = 30 } }
                        snap.world.map, snap.world.x, snap.world.y = 22, 30, 28
                        bound.direct(me, { id = "w", map = 22, steps = steps, done = 1, members = { me } })
                        local walks  = traffic.follow(snap, bound)
                        snap.world.y = 29
                        local ends   = traffic.follow(snap, bound)
                        local left   = bound.directed(me)
                        bound.direct(me, nil)
                        local dest = walks ~= nil and walks.dest or {}
                        return walks ~= nil and walks.kind == ACTION.MOVE and dest.x == 30 and dest.y == 29 and
                            ends == nil and
                            left == nil,
                            string.format("walks=%s dest=%s,%s ends=%s left=%s", tostring(walks and walks.kind),
                                tostring(dest.x), tostring(dest.y), tostring(ends), tostring(left))
                    end),
                    case("a poet on a plan heals its falling vanguard before it walks the plan's next step", function()
                        local main = require("brain.trees.main")
                        local function picked(hp)
                            local snap, bound = state("poet94")
                            local mate = state("warrior93", { hp = hp })
                            local me = snap.self.name
                            local other = "untangle-peer"
                            snap.world.map, snap.world.x, snap.world.y = 2376, 19, 14
                            mate.world.map, mate.world.x, mate.world.y = 2376, 16, 14
                            snap.buffs = { "무장", "보호" }
                            mate.buffs = { "무장", "보호" }
                            local a = mob_named("자해골", 15, 14, 901); a.attack = "AGGRESSIVE"; a.ratio = 0.55
                            local b = mob_named("흑해골", 16, 15, 902); b.attack = "AGGRESSIVE"; b.ratio = 0.37
                            snap.world.mobs, snap.world.occupied = { a, b }, {}
                            snap.party = { party.profile(mate, bound, nil) }
                            snap.party[1].task = "DEFEND"
                            snap.party[1].battle = { map = 2376, oid = 901 }
                            snap.party[1].peril = {
                                drain = 120,
                                drains = { ["901"] = 60, ["902"] = 60 },
                                hp = (a.hp or 0) * a.ratio + (b.hp or 0) * b.ratio,
                                dps = 300,
                                attackers = { 901, 902 },
                            }
                            snap.group = { mate.self.name, me }
                            snap.leader = mate.self.name
                            bound.ally = function(name)
                                if name == mate.self.name then
                                    return { oid = 9001, x = 16, y = 14, percent = 100 * hp // mate.self.max_hp }
                                elseif name == other then
                                    return { oid = 9002, x = 21, y = 13 }
                                end
                                return nil
                            end
                            bound.direct(me, {
                                id = "f",
                                map = 2376,
                                steps = { { x = 19, y = 14 }, { x = 20, y = 14 } },
                                done = 1,
                                members = { me, other }
                            })
                            bound.direct(other,
                                {
                                    id = "f",
                                    map = 2376,
                                    steps = { { x = 21, y = 13 }, { x = 22, y = 13 } },
                                    done = 1,
                                    members = { me, other }
                                })
                            local found = require("brain.tree").run(main, snap, bound)
                            bound.direct(me, nil)
                            bound.direct(other, nil)
                            return found ~= nil and found.name or "nil"
                        end
                        local falling = picked(703)
                        local whole   = picked(state("warrior93").self.max_hp)
                        return falling == "AID" and whole == "TRAFFIC",
                            string.format("falling=%s whole=%s", falling, whole)
                    end),
                    case("the tile beside the way down to a ground is a lane, and a member waits for its crew off it", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = state("mage95")
                        local beside                               = bound.lane(22, 31, 29)
                        local open                                 = bound.lane(22, 31, 28)
                        local door                                 = { x = 30, y = 30 }
                        snap.world.map, snap.world.x, snap.world.y = 22, 31, 29
                        snap.world.mobs                            = {}
                        local moved                                = together.waiting(snap, bound, door)
                        local dest                                 = moved ~= nil and moved.dest or {}
                        local off                                  = dest.x ~= nil and
                        bound.lane(22, dest.x, dest.y) == false
                        snap.world.x, snap.world.y                 = 31, 28
                        local stays                                = together.waiting(snap, bound, door)
                        return beside and open == false and off and stays == nil,
                            string.format("beside=%s open=%s dest=%s,%s stays=%s", tostring(beside), tostring(open),
                                tostring(dest.x), tostring(dest.y), tostring(stays))
                    end),
                    case("two crews tangled in the way down to a ground are untangled in one plan every bot walks", function()
                        local snap, bound = state("warrior93")
                        local movers      = {
                            { x = 30, y = 25, gx = 30, gy = 30, range = 0, priority = 2 },
                            { x = 30, y = 26, range = 0, priority = 0 },
                            { x = 31, y = 29, range = 0, priority = 0 },
                            { x = 31, y = 27, gx = 31, gy = 30, range = 0, priority = 1 },
                        }
                        local paths       = bound.untangle(22, movers, {}, 64)
                        if paths == nil then
                            return false, "no plan"
                        end
                        local length = 0
                        for _, path in ipairs(paths) do
                            length = math.max(length, #path)
                        end
                        local function at(index, step)
                            local path = paths[index]
                            return path[math.min(step, #path)]
                        end
                        local sound = true
                        for step = 1, length do
                            local seen = {}
                            for index = 1, #paths do
                                local here = at(index, step)
                                local key  = here.x .. "," .. here.y
                                local warp = bound.warp(22, here.x, here.y)
                                sound      = sound and (seen[key] == nil or warp)
                                seen[key]  = true
                                if step > 1 then
                                    local before = at(index, step - 1)
                                    sound        = sound and
                                    math.abs(here.x - before.x) + math.abs(here.y - before.y) <= 1
                                    for other = 1, #paths do
                                        local there = at(other, step)
                                        local was   = at(other, step - 1)
                                        sound       = sound and (other == index or
                                            not (there.x == before.x and there.y == before.y and was.x == here.x and
                                                was.y == here.y and (here.x ~= before.x or here.y ~= before.y)))
                                    end
                                end
                            end
                        end
                        local lead  = paths[1][#paths[1]]
                        local rogue = paths[4][#paths[4]]
                        return sound and lead.x == 30 and lead.y == 30 and rogue.x == 31 and rogue.y == 30,
                            string.format("sound=%s steps=%d lead=%d,%d rogue=%d,%d", tostring(sound), length, lead.x,
                                lead.y, rogue.x, rogue.y)
                    end),
                    case("a bot walks its part of a plan step by step, waiting for the others to walk the step before", function()
                        local traffic                              = require("brain.policies.traffic")
                        local snap, bound                          = state("mage95")
                        snap.world.map, snap.world.x, snap.world.y = 22, 30, 26
                        local me                                   = snap.self.name
                        local other                                = "untangle-peer"
                        local steps                                = { { x = 30, y = 26 }, { x = 30, y = 26 }, { x = 29, y = 26 } }
                        local theirs                               = { { x = 30, y = 25 }, { x = 30, y = 26 }, { x = 30, y = 27 } }
                        local peer                                 = { x = 30, y = 25 }
                        bound.ally                                 = function(name)
                            return name == other and { oid = 9002, x = peer.x, y = peer.y } or nil
                        end
                        bound.direct(me, { id = "p", map = 22, steps = steps, done = 2, members = { me, other } })
                        bound.direct(other, { id = "p", map = 22, steps = theirs, done = 1, members = { me, other } })
                        local waits = traffic.follow(snap, bound)
                        peer.y      = 26
                        bound.direct(other, { id = "p", map = 22, steps = theirs, done = 2, members = { me, other } })
                        local walks = traffic.follow(snap, bound)
                        bound.direct(me, nil)
                        bound.direct(other, nil)
                        local dest = walks ~= nil and walks.dest or {}
                        return waits ~= nil and waits.kind == ACTION.HOLD and walks ~= nil and walks.kind == ACTION.MOVE and
                            dest.x == 29 and dest.y == 26,
                            string.format("waits=%s walks=%s dest=%s,%s", tostring(waits and waits.kind),
                                tostring(walks and walks.kind), tostring(dest.x), tostring(dest.y))
                    end),
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
                    case("reachable grounds follow the region of the map the bot stands in", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("rogue_no_food")
                        local target = { floors = { { map = 65000 } } }
                        local view = setmetatable({
                            reach = function(_, _, from)
                                return from.x == 1 and { [from.map] = true } or { [from.map] = true, [65000] = true }
                            end,
                            region = function(_, x)
                                return 65000 + x
                            end,
                        }, { __index = bound })
                        snap.world.x, snap.world.y = 1, 1
                        local enclosed = g.reachable(snap, view, target)
                        snap.world.x = 2
                        local open = g.reachable(snap, view, target)
                        return enclosed == false and open == true,
                            string.format("enclosed=%s open=%s", tostring(enclosed), tostring(open))
                    end),
                    case("one region of a map is asked for its reach once, wherever the bot stands in it", function()
                        local g = require("brain.policies.ground")
                        local snap, bound = recorded("rogue_no_food")
                        local target = { floors = { { map = 64999 } } }
                        local asked = 0
                        local view = setmetatable({
                            reach = function(_, _, from)
                                asked = asked + 1
                                return { [from.map] = true, [64999] = true }
                            end,
                            region = function()
                                return 64999
                            end,
                        }, { __index = bound })
                        snap.world.x, snap.world.y = 3, 3
                        local first = g.reachable(snap, view, target)
                        snap.world.x = 4
                        local second = g.reachable(snap, view, target)
                        return first == true and second == true and asked == 1, string.format("asked=%d", asked)
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
                    case("recent moments keep the last decisions and redecide to the same choice", function()
                        local moments     = require("sim.moments")
                        local trace       = require("sim.trace")
                        local snap, bound = recorded("vanguard_wall_prey")
                        local _, brain_   = shared()
                        local decision    = decide.choose(snap, bound, nil, nil)
                        local actor       = {
                            options = { recent = 3 },
                            session = {
                                actors = {},
                                occupants = {},
                                seed = 1,
                                roster = function()
                                    return {}
                                end
                            },
                            base    = { self = { name = snap.self.name } },
                            clock   = 0,
                        }
                        for index = 1, 5 do
                            actor.clock = index
                            moments.keep(actor, snap, bound, decision)
                        end
                        local path = "logs/brain_unit.moments"
                        moments.spill(actor, path)
                        local kept = {}
                        for line in io.lines(path) do
                            table.insert(kept, load(line)())
                        end
                        os.remove(path)
                        if #kept ~= 3 then
                            return false, "kept " .. #kept
                        end
                        local _, again = moments.redo(brain_, kept[3])
                        return kept[1].t == 3 and kept[3].t == 5 and kept[3].chose == trace.described(decision) and
                            trace.described(again) == kept[3].chose,
                            string.format("first=%s last=%s chose=%s again=%s", tostring(kept[1].t), tostring(kept[3].t),
                                tostring(kept[3].chose), trace.described(again))
                    end),
                    case("a broken rule is kept from its first moment to its last, each break on its own", function()
                        local rules   = require("sim.world.rules")
                        local emitted = {}
                        local actor   = {
                            clock   = 1000,
                            active  = { map = { id = 7 } },
                            options = {},
                            base    = { self = { name = "unit" } },
                            session = { actors = {} },
                            emit    = function(_, record)
                                table.insert(emitted, record)
                            end,
                        }
                        rules.note(actor, "apart", true)
                        actor.clock = 3000
                        rules.note(actor, "apart", true)
                        local open = rules.open(actor)
                        actor.clock = 4000
                        rules.note(actor, "apart", false)
                        actor.clock = 5000
                        rules.note(actor, "apart", true)
                        actor.clock = 5500
                        rules.flush(actor)
                        return #open == 1 and open[1].from == 1000 and #emitted == 2 and emitted[1].from == 1000 and
                            emitted[1].to == 4000 and emitted[2].from == 5000 and emitted[2].to == 5500 and
                            #rules.open(actor) == 0,
                            string.format("open=%d emitted=%d first=%s-%s", #open, #emitted,
                                tostring(emitted[1] and emitted[1].from), tostring(emitted[1] and emitted[1].to))
                    end),
                    case("a striker's damage spell at a mob on its poet is a draw, at any other mob it is recorded", function()
                        local rules    = require("sim.world.rules")
                        local snap     = state("warrior63")
                        local poet     = {
                            me     = { x = snap.world.x + 2, y = snap.world.y, dead = false },
                            spells = {},
                            base   = { self = { name = "poet" } },
                        }
                        local mob      = { x = snap.world.x + 3, y = snap.world.y, locked = poet.me, dead = false }
                        local world    = { map = { id = snap.world.map }, mobs = { [9] = mob } }
                        local actor    = {
                            clock   = 0,
                            me      = { x = snap.world.x, y = snap.world.y, dead = false },
                            active  = world,
                            spells  = snap.spells,
                            options = {},
                            base    = { self = { name = snap.self.name } },
                            emit    = function()
                            end,
                        }
                        local group    = { members = { [actor] = true, [poet] = true } }
                        actor.session  = {
                            actors   = { actor, poet },
                            profiles = {},
                            group    = function()
                                return group
                            end,
                        }
                        poet.active    = world
                        local decision = { kind = ACTION.ATTACK, key = "백열장", oid = 9, task = "FIELD" }
                        rules.check(actor, snap, decision)
                        local drawing = #rules.open(actor)
                        mob.locked    = nil
                        rules.check(actor, snap, decision)
                        local open = rules.open(actor)
                        return drawing == 0 and #open == 1 and open[1].rule == "strike_spell",
                            string.format("drawing=%d open=%d rule=%s", drawing, #open,
                                tostring(open[1] and open[1].rule))
                    end),
                    case("the crewmate with the greatest heal for others is the healer, whatever small heal the others know", function()
                        local rules    = require("sim.world.rules")
                        local snap     = state("poet65")
                        local warrior  = {
                            me     = { x = snap.world.x - 1, y = snap.world.y, dead = false },
                            spells = state("warrior63").spells,
                            base   = { self = { name = "warrior" } },
                        }
                        local actor    = {
                            clock   = 0,
                            me      = { x = snap.world.x, y = snap.world.y, dead = false },
                            spells  = snap.spells,
                            options = {},
                            base    = { self = { name = snap.self.name } },
                            emit    = function()
                            end,
                        }
                        local mob      = { x = snap.world.x + 1, y = snap.world.y, locked = actor.me, dead = false }
                        local world    = { map = { id = snap.world.map }, mobs = { [9] = mob } }
                        local group    = { members = { [actor] = true, [warrior] = true } }
                        actor.active   = world
                        warrior.active = world
                        actor.session  = {
                            actors   = { actor, warrior },
                            profiles = {},
                            group    = function()
                                return group
                            end,
                        }
                        rules.check(actor, snap, { kind = ACTION.ATTACK, key = "화염주", oid = 9, task = "DEFEND" })
                        local open = {}
                        for _, entry in ipairs(rules.open(actor)) do
                            open[entry.rule] = true
                        end
                        return open.healer_struck == true and open.fuel_spell == true and open.strike_spell == nil,
                            string.format("healer_struck=%s fuel_spell=%s strike_spell=%s", tostring(open.healer_struck),
                                tostring(open.fuel_spell), tostring(open.strike_spell))
                    end),
                    case("a mage whose heal is weaker than its attack spell is no healer for the record", function()
                        local rules    = require("sim.world.rules")
                        local snap     = state("mage60")
                        local warrior  = {
                            me     = { x = snap.world.x - 1, y = snap.world.y, dead = false },
                            spells = state("warrior63").spells,
                            base   = { self = { name = "warrior" } },
                        }
                        local actor    = {
                            clock   = 0,
                            me      = { x = snap.world.x, y = snap.world.y, dead = false },
                            spells  = snap.spells,
                            options = {},
                            base    = { self = { name = snap.self.name } },
                            emit    = function()
                            end,
                        }
                        local mob      = { x = snap.world.x + 1, y = snap.world.y, locked = actor.me, dead = false }
                        local world    = { map = { id = snap.world.map }, mobs = { [9] = mob } }
                        local group    = { members = { [actor] = true, [warrior] = true } }
                        actor.active   = world
                        warrior.active = world
                        actor.session  = {
                            actors   = { actor, warrior },
                            profiles = {},
                            group    = function()
                                return group
                            end,
                        }
                        rules.check(actor, snap, { kind = ACTION.ATTACK, key = "화염주", oid = 9, task = "DEFEND" })
                        local open = {}
                        for _, entry in ipairs(rules.open(actor)) do
                            open[entry.rule] = true
                        end
                        return open.healer_struck == nil and open.fuel_spell == nil,
                            string.format("healer_struck=%s fuel_spell=%s", tostring(open.healer_struck),
                                tostring(open.fuel_spell))
                    end),
                    case("a vanguard takes the skeletons a few steps round from its poet before it leaves the floor", function()
                        local snap, bound = recorded("vanguard_leaves_near_skeletons")
                        local near        = { [79] = true, [26] = true, [65] = true, [43] = true }
                        local plan        = party.plan(snap, bound)
                        local targets     = {}
                        for _, floor in ipairs(plan.floors) do
                            if floor.map == snap.world.map then
                                targets = floor.targets
                            end
                        end
                        local found = require("brain.tasks.field").target(snap, bound, targets)
                        data.trace  = true
                        decide.choose(snap, bound, nil, nil)
                        local rows = decide.offers[snap.self.name] or {}
                        data.trace = false
                        local path = rows[1] ~= nil and rows[1].path or ""
                        return found ~= nil and near[found.mob.oid] == true and
                            string.find(path, "next_floor", 1, true) == nil,
                            string.format("prey=%s path=%s", tostring(found and found.mob.oid), path)
                    end),
                    case("a vanguard takes no prey across a wall from its poet, though the wall is within sight", function()
                        local snap, bound = recorded("vanguard_wall_prey")
                        local poet        = snap.party[1]
                        local escort      = require("brain.policies.escort")
                        local across      = 0
                        for _, mob in ipairs(snap.world.mobs) do
                            local seen  = math.abs(mob.x - poet.world.x) <= data.sight.x and
                                math.abs(mob.y - poet.world.y) <= data.sight.y
                            local steps = escort.steps(snap, bound, party.members(snap)[1], { { x = mob.x, y = mob.y } })
                                [1]
                            if seen and type(steps) == "number" and steps > data.sight.x + data.sight.y then
                                across = across + 1
                            end
                        end
                        if across == 0 then
                            return false, "premise: no prey across a wall in the poet's sight"
                        end
                        local plan   = party.plan(snap, bound)
                        local picked = require("brain.tasks.field").target(snap, bound, plan.floors[1].targets)
                        local walk   = picked ~= nil and
                            escort.steps(snap, bound, party.members(snap)[1], { { x = picked.mob.x, y = picked.mob.y } })
                            [1] or nil
                        return picked == nil or (type(walk) == "number" and walk <= math.min(data.sight.x, data.sight.y)),
                            string.format("across=%d picked=%s walk=%s", across, tostring(picked and picked.mob.oid),
                                tostring(walk))
                    end),
                    case("a caster shoots a mob across a wall from beside its poet, where a hitter would walk round", function()
                        local wall, seen          = recorded("vanguard_wall_prey")
                        local snap, bound         = state("mage60")
                        local targets             = party.plan(wall, seen).floors[1].targets
                        snap.world                = wall.world
                        snap.party                = wall.party
                        snap.group                = wall.group
                        snap.leader               = wall.leader
                        snap.record.memory.commit = wall.record.memory.commit
                        snap.self.mp              = snap.self.max_mp
                        local escort              = require("brain.policies.escort")
                        local picked              = require("brain.tasks.field").target(snap, bound, targets)
                        local walk                = picked ~= nil and
                            escort.steps(snap, bound, party.members(snap)[1], { { x = picked.mob.x, y = picked.mob.y } })
                            [1] or nil
                        return picked ~= nil and picked.choice.approach ~= true and type(walk) == "number" and
                            walk > data.sight.x + data.sight.y,
                            string.format("picked=%s key=%s approach=%s walk=%s", tostring(picked and picked.mob.oid),
                                tostring(picked and picked.choice.key), tostring(picked and picked.choice.approach),
                                tostring(walk))
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
                    case("walker kept off by a threat that stays is not held by another one moving elsewhere", function()
                        local walker = threatened_walk(9, 5, { from = { 2, 2 }, speed = 1000 })
                        local other  = { x = 8, y = 0, data = { id = 7, speed = 1000 } }
                        walker.active.map:enter(other)
                        walker.active.mobs[2] = other
                        local ticks           = 0
                        walker.advance        = function(self, duration)
                            self.clock = self.clock + duration
                            ticks      = ticks + 1
                            if ticks <= 10 then
                                self.active.map:shift(other, 8, ticks % 2)
                            end
                        end
                        local walked, reason  = walker:walk({ map = 1, x = 1, y = 2, range = 0 }, { [7] = true })
                        return walked == false and reason == "THREATENED" and walker.clock == 1000,
                            string.format("%s:%s:clock=%d", tostring(walked), reason, walker.clock)
                    end),
                    case("an approach goes no way round bodies longer than its walk by more than the leeway", function()
                        local function approach(rows)
                            local walker         = threatened_walk(9, 5, { from = { 6, 0 } })
                            walker.me.x          = 0
                            walker.me.y          = 0
                            walker.active.killed = {}
                            for y = 0, rows - 1 do
                                walker.active.map:enter({ x = 3, y = y })
                            end
                            return walker:engage(1, 5, 7), walker.me.x, walker.me.y
                        end
                        local held, hx, hy = approach(4)
                        local gone, gx, gy = approach(2)
                        return held == "UNREACHABLE" and hx == 0 and hy == 0 and gone == "APPROACHED" and gx + gy == 5,
                            string.format("held=%s at %d,%d gone=%s at %d,%d", tostring(held), hx, hy, tostring(gone), gx,
                                gy)
                    end),
                    case("a strike walked to carries the leeway of a walk", function()
                        local snap, bound = recorded("strike_round_the_bodies")
                        local pick        = decide.choose(snap, bound, nil, nil)
                        local maneuver    = pick ~= nil and pick.maneuver or {}
                        return pick ~= nil and pick.kind == ACTION.ATTACK and (maneuver.steps or 0) > 1 and
                            maneuver.leeway == require("brain.policies.escort").reach(),
                            string.format("kind=%s key=%s steps=%s leeway=%s", tostring(pick and pick.kind),
                                tostring(pick and pick.key),
                                tostring(maneuver.steps), tostring(maneuver.leeway))
                    end),
                    case("a leap that lands at random takes a tile its door leads to", function()
                        local enums   = require("sim.world.runtime").enums()
                        local closed  = "\0\1\0\0\0"
                        local map     = require("sim.world.map").new(1,
                            { width = 5, height = 1, blocked = closed, collisions = string.rep("\0", 5) }, enums)
                        local world   = {
                            map = map,
                            place = function(_, object, x, y)
                                object.x, object.y = x, y
                            end,
                        }
                        local actor   = setmetatable({
                            me = { stats = { kills = 0, taken = 0, swings = 0, hits = 0 }, exp = 0 },
                            clock = 0,
                            visits = {},
                            base = { world = { map = 2, x = 0, y = 0 }, self = { level = 1 } },
                            session = {
                                profiles = {},
                                world = function()
                                    return world
                                end,
                            },
                            mind = {
                                route = function()
                                    return { dest = 1, x = 3, y = 0 }
                                end,
                                region = function(_, _, x)
                                    return x == 0 and 1 or (x >= 2 and 2 or 0)
                                end,
                            },
                        }, { __index = driver.Actor })
                        local random  = math.random
                        math.random   = function()
                            return 1
                        end
                        local entered = actor:enter(1)
                        math.random   = random
                        return entered == true and actor.me.x == 2,
                            string.format("%s at %s", tostring(entered), tostring(actor.me.x))
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
                    case("a charge is weighed by its chance of success", function()
                        local rest     = require("brain.tasks.rest")
                        local recovery = require("brain.policies.recovery")
                        local function weighed(int)
                            local snap, bound = state("poet65", { mp = 35, hp = 2400, int = int })
                            local slot        = recovery.spells(snap).charge
                            return rest.worth(snap, bound, { kind = ACTION.CAST, slot = slot.slot }, false)
                        end
                        local plain = weighed(65)
                        local surer = weighed(255)
                        return plain > 0 and surer > plain, string.format("int65=%.1f int255=%.1f", plain, surer)
                    end),
                    case("a party plan tells a member refilled from dry whichever is asked first", function()
                        local function team()
                            local snap, bound                          = state("rogue50", { mp = 735, money = 70300 })
                            local poet                                 = state("poet65")
                            snap.world.map, snap.world.x, snap.world.y = 863, 13, 12
                            poet.world.map, poet.world.x, poet.world.y = 863, 14, 12
                            snap.party                                 = { party.profile(poet, bound, nil) }
                            snap.group                                 = { snap.self.name, poet.self.name }
                            snap.leader                                = snap.self.name
                            local members                              = { snap }
                            for _, member in ipairs(party.members(snap)) do
                                table.insert(members, member)
                            end
                            return members, bound
                        end
                        local first, bound  = team()
                        local refill        = ground.party(first, bound, "REFILL")
                        local dry           = ground.party(first, bound, "DRY")
                        local second, again = team()
                        local dried         = ground.party(second, again, "DRY")
                        local refilled      = ground.party(second, again, "REFILL")
                        if refill == nil or dry == nil or dried == nil or refilled == nil then
                            return false, "no plan"
                        end
                        local detail = string.format("refill=%.1f/%.1f dry=%.1f/%.1f", refill.rate, refilled.rate,
                            dry.rate, dried.rate)
                        local same = refill.rate == refilled.rate and dry.rate == dried.rate
                        return same and refill.rate > dry.rate, detail
                    end),
                    case("an ambush jumps a crewmate in the way, behind it or else beside it, never onto a warp", function()
                        local snap, bound = state("rogue50")
                        local facts = setmetatable({
                            floor = function()
                                return true
                            end,
                            warp = function()
                                return false
                            end,
                        }, { __index = bound })
                        local mate = state("poet65", { name = "vault_mate" })
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                        snap.party = { party.profile(mate, bound, nil) }
                        local behind = battle.vault(snap, facts, mate.world.x, mate.world.y)
                        table.insert(snap.world.mobs, mob_named("흑해골", snap.world.x + 2, snap.world.y, 901))
                        local beside = battle.vault(snap, facts, mate.world.x, mate.world.y)
                        local warped = battle.vault(snap, setmetatable({
                            warp = function()
                                return true
                            end,
                        }, { __index = facts }), mate.world.x, mate.world.y)
                        return behind ~= nil and behind.x == snap.world.x + 2 and behind.y == snap.world.y and
                            beside ~= nil and beside.x == snap.world.x + 1 and beside.y == snap.world.y - 1 and
                            warped == nil,
                            string.format("behind=%s,%s beside=%s,%s warped=%s", tostring(behind and behind.x),
                                tostring(behind and behind.y), tostring(beside and beside.x),
                                tostring(beside and beside.y),
                                tostring(warped ~= nil))
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
                        local mob = surround(snap, "양철인형", 4)
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
                    case("a mana dose is bought by what a refill costs: the cheapest for a few drinks, a big one for many", function()
                        local supply = require("brain.tasks.supply")
                        local function dry(name, round, drop)
                            local snap, bound = state(name)
                            for _, spell in ipairs(drop or {}) do
                                without(snap, spell)
                            end
                            local kept = {}
                            for _, item in ipairs(snap.bag.items) do
                                if (items[item.name] or {}).mp == nil then
                                    table.insert(kept, item)
                                end
                            end
                            snap.bag.items                          = kept
                            snap.bag.used                           = #kept
                            snap.pace.use                           = round
                            snap.self.mp                            = 0
                            local choice, _, _, _, _, _, _, _, kind = supply.order(snap, bound)
                            return kind == "mp" and choice ~= nil and choice.name or tostring(kind)
                        end
                        local rogue = dry("rogue92", 60)
                        local timed = dry("poet94", 60, { "공력증강" })
                        local free  = dry("poet94", nil, { "공력증강" })
                        return rogue == "동동주" and timed ~= "동동주" and timed ~= "mp" and free == "동동주",
                            string.format("rogue=%s poet=%s poet_untimed=%s", rogue, timed, free)
                    end),
                    case("a strike that spends all the mana stocks doses past one 동동주", function()
                        local supply = require("brain.tasks.supply")
                        local function rogue(doses, shop)
                            local snap, bound = state("rogue92")
                            local dose        = nil
                            local kept        = {}
                            for _, item in ipairs(snap.bag.items) do
                                if (items[item.name] or {}).mp == nil then
                                    table.insert(kept, item)
                                elseif item.name == "동동주" then
                                    dose = item
                                end
                            end
                            for _, uses in ipairs(doses) do
                                table.insert(kept, setmetatable({ count = uses }, { __index = dose }))
                            end
                            snap.bag.items = kept
                            snap.bag.used  = #kept
                            snap.pace.use  = 60
                            snap.self.mp   = 0
                            if shop ~= nil then
                                snap.world.map = shop
                            end
                            local choice, _, _, dest, _, _, _, _, kind = supply.order(snap, bound)
                            return kind == "mp" and choice ~= nil and choice.name or nil, dest
                        end
                        local _, shop = rogue({})
                        local more    = rogue({ 200, 200, 200 }, shop)
                        return shop ~= nil and more == "동동주",
                            string.format("shop=%s more=%s", tostring(shop), tostring(more))
                    end),
                    case("a crewmate heals the others when its heals outdo its damage, not for knowing a heal", function()
                        local score        = require("brain.policies.score")
                        local rogue, bound = state("rogue92")
                        local poet         = state("poet94")
                        local warrior      = state("warrior93")
                        local heals        = score.capability(rogue, bound).hps.others
                        return heals > 0 and score.healer(rogue, bound) == false and score.healer(poet, bound) and
                            score.healer(warrior, bound) == false,
                            string.format("rogue heals=%.0f healer=%s poet=%s warrior=%s", heals,
                                tostring(score.healer(rogue, bound)), tostring(score.healer(poet, bound)),
                                tostring(score.healer(warrior, bound)))
                    end),
                    case("a strike that spends hp is cast while it takes more of the mob than it stakes of me: at full hp, not from a third", function()
                        local battle = require("brain.policies.battle")
                        local function pick(hp, count)
                            local snap, bound = state("warrior93", { hp = hp })
                            local poet        = state("poet94")
                            local rogue       = state("rogue92", { hp = 3000 })
                            local kept        = {}
                            for _, item in ipairs(snap.bag.items) do
                                if (items[item.name] or {}).hp == nil then
                                    table.insert(kept, item)
                                end
                            end
                            snap.bag.items               = kept
                            poet.world.x, poet.world.y   = snap.world.x - 2, snap.world.y
                            rogue.world.x, rogue.world.y = snap.world.x + 2, snap.world.y
                            snap.party                   = { party.profile(poet, bound, nil), party.profile(rogue, bound,
                                nil) }
                            snap.group                   = { snap.self.name, poet.self.name, rogue.self.name }
                            snap.leader                  = snap.self.name
                            local found                  = battle.choose(snap, bound, surround(snap, "지해골", count), true)
                            return tostring(found and found.key)
                        end
                        local full, worn, third, crowd = pick(6972, 3), pick(5000, 1), pick(2291, 1), pick(2291, 3)
                        return full == "건곤대나이" and worn == "건곤대나이" and third ~= "건곤대나이" and crowd ~= "건곤대나이",
                            string.format("full=%s worn=%s third=%s crowd=%s", full, worn, third, crowd)
                    end),
                    case("a ground held by the crew keeps its crowd, so the buffs that strike beside the target are cast on it", function()
                        local buff                                    = require("brain.tasks.buff")
                        local snap, bound                             = state("warrior93", { mp = 1309 })
                        local poet                                    = state("poet94")
                        local rogue                                   = state("rogue92")
                        snap.world.map, snap.world.x, snap.world.y    = 22, 16, 103
                        poet.world.map, poet.world.x, poet.world.y    = 22, 15, 103
                        rogue.world.map, rogue.world.x, rogue.world.y = 22, 17, 103
                        snap.party                                    = { party.profile(poet, bound, nil), party.profile(
                            rogue, bound, nil) }
                        snap.group                                    = { snap.self.name, poet.self.name, rogue.self
                            .name }
                        snap.leader                                   = snap.self.name
                        snap.record.memory.commit                     = "흑해골굴"
                        snap.buffs                                    = {}
                        local cast                                    = {}
                        for _ = 1, 6 do
                            local best = buff.best(snap, bound)
                            if best == nil or best.value < 0 then
                                break
                            end
                            cast[best.name] = true
                            table.insert(snap.buffs, best.name)
                        end
                        local plan = party.plan(snap, bound)
                        return plan ~= nil and plan.crowd ~= nil and cast["측면공격"] == true and cast["후면공격"] == true,
                            string.format("plan=%s crowd=%s cast=%s", tostring(plan and plan.name),
                                tostring(plan and plan.crowd),
                                table.concat(snap.buffs, ","))
                    end),
                    case("a mage among four mobs casts the spell that hits them all and beside one the single", function()
                        local battle = require("brain.policies.battle")
                        local function picked(count)
                            local snap, bound                          = state("mage91", { hp = 3930, mp = 2983 })
                            snap.world.map, snap.world.x, snap.world.y = 5375, 9, 4
                            local spots                                = { { "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 8, 4 }, { "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 9, 5 }, { "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 10, 4 }, { "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 9, 3 } }
                            local mobs                                 = {}
                            for index = 1, count do
                                local spot = spots[index]
                                local mob  = mob_named(spot[1], spot[2], spot[3], 900 + index)
                                mob.attack = "AGGRESSIVE"
                                table.insert(mobs, mob)
                            end
                            snap.world.mobs = mobs
                            local found = battle.choose(snap, bound, mobs[1], true)
                            return found ~= nil and found.key or "nil"
                        end
                        local four = picked(4)
                        local one  = picked(1)
                        return four ==
                        "\xea\xb7\xb9\xec\xa7\x84\xed\x99\x94\xec\x97\xb4\xec\xb0\xb8\xec\xa3\xbc\x27\xec\xb2\xa8" and
                        one ~= four, string.format("four=%s one=%s", four, one)
                    end),
                    case("a leap short of mana drinks the least dose that makes it up, and with no dose it is no leap", function()
                        local go = require("brain.trees.go")
                        local function leap(snap)
                            for _, jump in ipairs(go.jumps(snap, false)) do
                                if jump.spell == "비영사천문" then
                                    return jump
                                end
                            end
                            return nil
                        end
                        local full    = state("poet65")
                        full.self.mp  = 30
                        local short   = state("poet65")
                        short.self.mp = 15
                        local dry     = state("poet65")
                        dry.self.mp   = 15
                        dry.bag.items = {}
                        local kept    = leap(full)
                        local fueled  = leap(short)
                        local dose    = fueled ~= nil and fueled.fuel ~= nil and items[fueled.fuel] or nil
                        return kept ~= nil and kept.fuel == nil and dose ~= nil and (dose.mp or 0) >= 15 and
                            leap(dry) == nil,
                            string.format("full=%s fuel=%s dry=%s", tostring(kept and kept.fuel),
                                tostring(fueled and fueled.fuel), tostring(leap(dry) ~= nil))
                    end),
                    case("a walk counted in more steps than a leap takes weighs leaps, a walk of fewer does not", function()
                        local go          = require("brain.trees.go")
                        local snap, bound = state("poet65")
                        local mock        = setmetatable({
                            leaps = function()
                                return nil
                            end,
                        }, { __index = bound })
                        local cost        = math.ceil(data.transfer / snap.pace.step)
                        local function weighed(steps)
                            local detour = { closed = {}, perilous = {}, shunned = {}, avoid = {}, spent = {} }
                            go.choose(snap, mock,
                                { map = snap.world.map, x = snap.world.x + 40, y = snap.world.y, range = 1, steps = steps },
                                detour)
                            return #detour.jumps
                        end
                        local long, short = weighed(cost + 1), weighed(cost)
                        return long > 0 and short == 0, string.format("long=%d short=%d", long, short)
                    end),
                    case("a way that runs on through the ground to a shop past it is no leaving and takes no leap", function()
                        local snap, bound = recorded("supply_through_ground")
                        local detour      = { closed = {}, perilous = {}, shunned = {}, avoid = {}, spent = {} }
                        local leap        = require("brain.trees.go").choose(snap, bound, { maps = { 929 } }, detour)
                        return leap == nil and detour.leaving == false and #detour.jumps > 0,
                            string.format("leap=%s leaving=%s jumps=%d", tostring(leap and leap.name),
                                tostring(detour.leaving),
                                #detour.jumps)
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
                    case("a member that has not moved since its walk failed is pinned while it drinks, not for want of a way or a character in it", function()
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
                        snap.record.memory.blocked = { map = snap.world.map, at = snap.now - 1000, walls = true }
                        local walled               = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.MOVE, dest = { maps = { snap.world.map } } })
                        snap.record.memory.blocked = { map = snap.world.map, at = snap.now - 1000, detail = "CHARACTER" }
                        local crowded              = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.MOVE, dest = { maps = { snap.world.map } } })
                        snap.record.memory.blocked = { map = snap.world.map, at = snap.now - 1000, detail = "NPC" }
                        local npc                  = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.MOVE, dest = { maps = { snap.world.map } } })
                        return drinking.pinned == true and walking.pinned == true and walled.pinned == nil and
                            crowded.pinned == nil and npc.pinned == nil,
                            string.format("drinking=%s walking=%s walled=%s crowded=%s npc=%s", tostring(drinking.pinned),
                                tostring(walking.pinned), tostring(walled.pinned), tostring(crowded.pinned),
                                tostring(npc.pinned))
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
                    case("a poet heals a mage its bar shows whole when the mage posts the hp a spell left it", function()
                        local aid = require("brain.tasks.aid")
                        local function healed(posted)
                            local snap, bound                          = state("poet94")
                            local mate                                 = state("mage91")
                            snap.world.map, snap.world.x, snap.world.y = 5375, 9, 6
                            mate.world.map, mate.world.x, mate.world.y = 5375, 9, 4
                            mate.self.hp                               = posted
                            snap.world.mobs                            = {}
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "FIELD"
                            snap.group                                 = { mate.self.name, snap.self.name }
                            snap.leader                                = mate.self.name
                            bound.ally                                 = function(name)
                                return name == mate.self.name and { oid = 9001, x = 9, y = 4, percent = 100 } or nil
                            end
                            return aid.worth("heal")(snap, bound)
                        end
                        local spent = healed(100)
                        local whole = healed(state("mage91").self.max_hp)
                        return spent ~= nil and spent > 0 and (whole == nil or whole < spent),
                            string.format("spent=%s whole=%s", tostring(spent), tostring(whole))
                    end),
                    case("a mage holds a mob off its warrior beset on four sides at low hp, not off a whole warrior by one", function()
                        local aid = require("brain.tasks.aid")
                        local function offered(hp, count)
                            local snap, bound                          = state("mage91")
                            local mate                                 = state("warrior93")
                            snap.world.map, snap.world.x, snap.world.y = 5375, 9, 8
                            mate.world.map, mate.world.x, mate.world.y = 5375, 9, 5
                            mate.self.hp                               = hp
                            local spots                                = { { "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 8, 5 }, { "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 10, 5 }, { "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 9, 4 }, { "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 9, 6 } }
                            local mobs                                 = {}
                            for index = 1, count do
                                local spot = spots[index]
                                local mob  = mob_named(spot[1], spot[2], spot[3], 900 + index)
                                mob.attack = "AGGRESSIVE"
                                table.insert(mobs, mob)
                            end
                            snap.world.mobs      = mobs
                            snap.party           = { party.profile(mate, bound, nil) }
                            snap.party[1].task   = "DEFEND"
                            snap.party[1].battle = { map = 5375, oid = 901 }
                            snap.group           = { mate.self.name, snap.self.name }
                            snap.leader          = mate.self.name
                            bound.ally           = function(name)
                                return name == mate.self.name and
                                    { oid = 9001, x = 9, y = 5, percent = 100 * hp / mate.self.max_hp } or nil
                            end
                            local found          = aid.offer("debuff")(snap, bound)
                            return found ~= nil and found.key or "nil", aid.worth("debuff")(snap, bound)
                        end
                        local beset_key, beset = offered(state("warrior93").self.max_hp // 5, 4)
                        local whole_key, whole = offered(state("warrior93").self.max_hp, 1)
                        local holds            = beset_key == "\xeb\xa7\x88\xeb\xb9\x84" or
                        beset_key == "\xec\xa0\x88\xeb\xa7\x9d"
                        return
                            holds and
                            ((whole_key ~= "\xeb\xa7\x88\xeb\xb9\x84" and whole_key ~= "\xec\xa0\x88\xeb\xa7\x9d") or (whole or 0) < beset),
                            string.format("beset=%s/%s whole=%s/%s", beset_key, tostring(beset), whole_key,
                                tostring(whole))
                    end),
                    case("a mage alone among four mobs holds one off itself", function()
                        local aid                                  = require("brain.tasks.aid")
                        local snap, bound                          = state("mage91", { hp = 1500, mp = 2983 })
                        snap.world.map, snap.world.x, snap.world.y = 5375, 9, 4
                        local spots                                = { { "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 8, 4 }, { "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 9, 5 }, { "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 10, 4 }, { "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 9, 3 } }
                        local mobs                                 = {}
                        for index, spot in ipairs(spots) do
                            local mob  = mob_named(spot[1], spot[2], spot[3], 900 + index)
                            mob.attack = "AGGRESSIVE"
                            table.insert(mobs, mob)
                        end
                        snap.world.mobs = mobs
                        local found     = aid.offer("debuff")(snap, bound)
                        local key       = found ~= nil and found.key or "nil"
                        return key == "\xeb\xa7\x88\xeb\xb9\x84" or key == "\xec\xa0\x88\xeb\xa7\x9d", "key=" .. key
                    end),
                    case("a mage curses a mob beside a warrior but not beside a poet that casts 혼마술", function()
                        local aid = require("brain.tasks.aid")
                        local function offered(crewmate)
                            local snap, bound = state("mage91")
                            local mate        = state(crewmate)
                            local kept        = {}
                            for _, learned in ipairs(snap.spells) do
                                if learned.name == "\xec\xa0\x80\xec\xa3\xbc" then
                                    table.insert(kept, learned)
                                end
                            end
                            snap.spells                                = kept
                            snap.world.map, snap.world.x, snap.world.y = 5375, 9, 8
                            mate.world.map, mate.world.x, mate.world.y = 5375, 9, 5
                            local mob                                  = mob_named(
                            "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 9, 4, 901)
                            mob.attack                                 = "AGGRESSIVE"
                            snap.world.mobs                            = { mob }
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "DEFEND"
                            snap.party[1].battle                       = { map = 5375, oid = 901 }
                            snap.group                                 = { mate.self.name, snap.self.name }
                            snap.leader                                = mate.self.name
                            bound.ally                                 = function(name)
                                return name == mate.self.name and { oid = 9001, x = 9, y = 5, percent = 100 } or nil
                            end
                            local found                                = aid.offer("debuff")(snap, bound)
                            return found ~= nil and found.key or "nil"
                        end
                        local poet    = offered("poet94")
                        local warrior = offered("warrior93")
                        return poet == "nil" and warrior == "\xec\xa0\x80\xec\xa3\xbc",
                            string.format("poet=%s warrior=%s", poet, warrior)
                    end),
                    case("a mage poisons the mobs in sight before they reach it, not one already poisoned", function()
                        local aid = require("brain.tasks.aid")
                        local function offered(poisoned)
                            local snap, bound                          = state("mage91")
                            snap.world.map, snap.world.x, snap.world.y = 5375, 9, 8
                            local mob                                  = mob_named(
                            "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 13, 3, 901)
                            mob.attack                                 = "AGGRESSIVE"
                            snap.world.mobs                            = { mob }
                            if poisoned then
                                snap.record.memory.marks = { ["901"] = { ["\xec\xa4\x91\xeb\x8f\x85"] = snap.now } }
                            end
                            local found = aid.offer("debuff")(snap, bound)
                            return found ~= nil and found.key or "nil", found ~= nil and found.oid or 0
                        end
                        local fresh, oid = offered(false)
                        local again      = offered(true)
                        return fresh == "\xec\xa4\x91\xeb\x8f\x85" and oid == 901 and again ~= "\xec\xa4\x91\xeb\x8f\x85",
                            string.format("fresh=%s/%s again=%s", fresh, tostring(oid), again)
                    end),
                    case("a mage struck hard by two mobs steps off before it strikes, and beside one weak mob strikes", function()
                        local function chosen(name, count)
                            local snap, bound                          = state("mage91")
                            snap.world.map, snap.world.x, snap.world.y = 5375, 9, 8
                            local spots                                = { { 9, 9 }, { 10, 8 } }
                            local mobs                                 = {}
                            for index = 1, count do
                                local mob  = mob_named(name, spots[index][1], spots[index][2], 900 + index)
                                mob.attack = "AGGRESSIVE"
                                mob.ratio  = 0.8
                                table.insert(mobs, mob)
                            end
                            snap.world.mobs  = mobs
                            snap.self.struck = 0
                            local step       = require("brain.trees.together").step_worth(snap, bound)
                            local strike     = require("brain.tasks.defend").value(snap, bound)
                            return step ~= nil and step > strike,
                                string.format("%s/%s", tostring(step), tostring(strike))
                        end
                        local pressed, how = chosen("\xec\x9d\xbc\xeb\xb3\xb8\xeb\xb0\x80\xec\xa0\x95\x27\xec\x9c\xa0", 2)
                        local weak, why    = chosen("\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 1)
                        return pressed and weak == false, string.format("pressed=%s weak=%s", how, why)
                    end),
                    case("a mage keeps its falling warrior up before it poisons the mobs in sight", function()
                        local support = require("brain.trees.support")
                        local function tended(hp, dps)
                            local snap, bound                          = state("mage91", { hp = 2499, mp = 2810 })
                            local mate                                 = state("warrior93", { hp = hp, mp = 83 })
                            snap.world.map, snap.world.x, snap.world.y = 5245, 15, 23
                            mate.world.map, mate.world.x, mate.world.y = 5245, 13, 23
                            snap.buffs                                 = { "무장", "보호" }
                            mate.buffs                                 = { "무장", "보호" }
                            local a                                    = mob_named(
                            "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 12, 23, 901); a.attack = "AGGRESSIVE"; a.ratio = 0.55
                            local b                                    = mob_named(
                            "\xec\xa7\x80\xed\x95\xb4\xea\xb3\xa8", 14, 22, 902); b.attack = "AGGRESSIVE"; b.ratio = 0.37
                            local far                                  = {
                                mob_named("\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 10, 18, 911), mob_named(
                            "\xec\xa3\xbc\xed\x95\xb4\xea\xb3\xa8", 7, 19, 912), mob_named(
                            "\xec\x9e\x90\xed\x95\xb4\xea\xb3\xa8", 6, 24, 913),
                                mob_named("\xec\xa7\x80\xed\x95\xb4\xea\xb3\xa8", 7, 21, 914), mob_named(
                            "\xed\x9d\x91\xed\x95\xb4\xea\xb3\xa8", 9, 26, 915),
                            }
                            snap.world.mobs                            = { a, b }
                            for _, mob in ipairs(far) do
                                mob.attack = "AGGRESSIVE"
                                table.insert(snap.world.mobs, mob)
                            end
                            snap.party           = { party.profile(mate, bound, nil) }
                            snap.party[1].task   = "DEFEND"
                            snap.party[1].battle = { map = 5245, oid = 901 }
                            local total          = (a.hp or 0) * a.ratio + (b.hp or 0) * b.ratio
                            snap.party[1].peril  = {
                                drain = 120,
                                drains = { ["901"] = 60, ["902"] = 60 },
                                hp = total,
                                dps = dps,
                                attackers = { 901, 902 },
                            }
                            snap.group           = { mate.self.name, snap.self.name }
                            snap.leader          = mate.self.name
                            bound.ally           = function(name)
                                return name == mate.self.name and
                                    { oid = 9001, x = 13, y = 23, percent = 100 * hp // mate.self.max_hp } or nil
                            end
                            local found          = require("brain.tree").run(support.tend, snap, bound)
                            local action         = found ~= nil and found.action or {}
                            return tostring(action.key), tostring(action.oid)
                        end
                        local falling, at = tended(1933, 300)
                        local whole       = tended(state("warrior93").self.max_hp, 2000)
                        return falling ~= "\xec\xa4\x91\xeb\x8f\x85" and falling ~= "nil" and
                            whole == "\xec\xa4\x91\xeb\x8f\x85",
                            string.format("falling=%s@%s whole=%s", falling, at, whole)
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
                    case("a poet beside its resting warrior strikes back for its own share without mana, alone or out of sight in full", function()
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
                            local defend        = require("brain.tasks.defend")
                            local found         = battle.target(snap, bound, function(mob)
                                return battle.attacking(snap, mob) and battle.struck(snap, mob)
                            end)
                            local whole         = found.choice.value
                            return defend.value(snap, bound) - math.max(0, whole), defend.strike_back(snap, bound).key
                        end
                        local full, full_key   = strike(100, 420)
                        local hurt, hurt_key   = strike(30, 420)
                        local alone, alone_key = strike(100, 22)
                        local far, far_key     = strike(30, 420, 37)
                        return full_key ~= "화염주" and hurt_key ~= "화염주" and full <= 1e-9 and hurt <= 1e-9 and
                            alone_key == "화염주" and far_key == "화염주",
                            string.format("full=%s/%s hurt=%s/%s alone=%s/%s far=%s/%s", tostring(full), full_key,
                                tostring(hurt), hurt_key, tostring(alone), alone_key, tostring(far), far_key)
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
                        return found ~= nil and found.choice.key ~= "화염주" and fuel > 0 and apart == 0 and struck == false,
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
                            snap.record.memory.entry     = { roster = ground.roster_of(snap.world.map), at = joined }
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
                    case("a member back at the entrance waits there while its leader inside is about to leave for servicing", function()
                        local snap, bound = recorded("member_enters_before_leader_leaves")
                        local lead        = party.leader(snap)
                        local plan        = party.plan(snap, bound)
                        local gate        = party.entry_gate(snap, bound, plan)
                        local before      = party.outbound(snap, gate)
                        local pick        = decide.choose(snap, bound, nil, nil)
                        local dest        = pick ~= nil and pick.dest or {}
                        local stays       = pick ~= nil and
                            (pick.kind == ACTION.HOLD or (dest.maps == nil and dest.map == snap.world.map))
                        lead.joined       = snap.now
                        local led         = party.outbound(snap, gate)
                        lead.joined       = nil
                        lead.servicing    = nil
                        lead.heading      = { gate }
                        local staging     = party.outbound(snap, gate)
                        return before == true and stays == true and led == false and staging == true,
                            string.format("outbound=%s kind=%s dest=%s/%s led=%s staging=%s", tostring(before),
                                tostring(pick and pick.kind), tostring(dest.map), tostring(dest.maps and dest.maps[1]),
                                tostring(led), tostring(staging))
                    end),
                    case("a leader on a floor past the entrance with its crew still outside goes to the entrance", function()
                        local probe, seen = apart_floors("warrior63", "poet65")
                        local gate        = party.plan(probe, seen).gate
                        local function pick(world)
                            local snap, bound = apart_floors("warrior63", "poet65",
                                world ~= nil and { world = world } or nil)
                            data.trace        = true
                            local found       = decide.choose(snap, bound, nil, nil)
                            data.trace        = false
                            local rows        = decide.offers[snap.self.name] or {}
                            local dest        = found ~= nil and found.dest or {}
                            return party.passing_through(snap, bound), rows[1] ~= nil and rows[1].path or "",
                                dest.maps ~= nil and dest.maps[1] or nil
                        end
                        local out, path, to  = pick({ map = gate, x = 1, y = 1 })
                        local inside, within = pick(nil)
                        return ground.floor_at(gate) == nil and out == true and
                            string.find(path, "to_entrance", 1, true) ~= nil and
                            to == gate and inside == false and string.find(within, "to_entrance", 1, true) == nil,
                            string.format("gate=%s out=%s path=%s to=%s inside=%s within=%s", tostring(gate),
                                tostring(out), path,
                                tostring(to), tostring(inside), within)
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
                        snap.record.memory.entry = { roster = ground.roster_of(snap.world.map), at = snap.now - 60000 }
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
                    case("a member coming back onto the floor after servicing does not walk straight out again", function()
                        local function path(entry)
                            local snap, bound            = apart_floors("poet65", "warrior63")
                            snap.record.memory.servicing = { at = snap.now - 5000, leave = true }
                            snap.record.memory.serviced  = nil
                            snap.record.memory.entry     = entry ~= nil and
                                { roster = ground.roster_of(snap.world.map), at = snap.now + entry } or nil
                            data.trace                   = true
                            decide.choose(snap, bound, nil, nil)
                            local rows = decide.offers[snap.self.name] or {}
                            data.trace = false
                            return rows[1] ~= nil and rows[1].path or ""
                        end
                        local entering = path(nil)
                        local inside   = path(-60000)
                        return string.find(entering, "leave_for_service", 1, true) == nil and
                            string.find(inside, "leave_for_service", 1, true) ~= nil,
                            string.format("entering=%s inside=%s", entering, inside)
                    end),
                    case("the vanguard waits for a crewmate at the edge of walking reach and walks once it closes in", function()
                        local escort = require("brain.policies.escort")
                        local function pick(x, y)
                            local snap, bound                          = apart_floors("warrior63", "poet65")
                            local mate                                 = snap.party[1]
                            snap.world.map, snap.world.x, snap.world.y = 421, 18, 6
                            mate.world.map, mate.world.x, mate.world.y = snap.world.map, x, y
                            mate.task                                  = "FIELD"
                            local plan                                 = party.plan(snap, bound)
                            local name                                 = nil
                            for _, floor in ipairs(plan.floors) do
                                if floor.map == snap.world.map then
                                    name = floor.targets[1]
                                end
                            end
                            snap.world.mobs = { mob_named(name, 22, 3, 777) }
                            local walk      = bound.steps(snap.world.map, x, y, { { x = 18, y = 6, range = 0 } })[1]
                            return decide.choose(snap, bound, nil, nil), walk
                        end
                        local far        = pick(14, 9)
                        local near, walk = pick(16, 8)
                        local dest       = near ~= nil and near.dest or {}
                        return far ~= nil and far.kind == ACTION.HOLD and far.leashed == true and near ~= nil and
                            near.kind == ACTION.MOVE and dest.steps == escort.reach() - walk,
                            string.format("far=%s/%s near=%s steps=%s walk=%s", tostring(far and far.kind),
                                tostring(far and far.leashed), tostring(near and near.kind), tostring(dest.steps),
                                tostring(walk))
                    end),
                    case("a member closes in to the vanguard waiting for it, counted in walking steps", function()
                        local escort                               = require("brain.policies.escort")
                        local snap, bound                          = apart_floors("poet65", "rogue50")
                        local lead                                 = snap.party[1]
                        lead.world.map, lead.world.x, lead.world.y = snap.world.map, snap.world.x + 6, snap.world.y
                        lead.leashed, lead.task                    = true, "FIELD"
                        local pick                                 = decide.choose(snap, bound, nil, nil)
                        local dest                                 = pick ~= nil and pick.dest or {}
                        local walk                                 = bound.steps(snap.world.map, snap.world.x,
                            snap.world.y, { { x = lead.world.x, y = lead.world.y, range = 0 } })[1]
                        local walled, seen                         = recorded("poet_behind_leashed_rogue")
                        local rogue                                = walled.party[1]
                        local round                                = escort.steps(walled, seen, walled,
                            { { x = rogue.world.x, y = rogue.world.y } })[1]
                        local fetch                                = decide.choose(walled, seen, nil, nil)
                        local goal                                 = fetch ~= nil and fetch.dest or {}
                        return pick ~= nil and pick.kind == ACTION.MOVE and dest.x == lead.world.x and
                            dest.y == lead.world.y and type(walk) == "number" and
                            dest.steps == math.min(escort.reach(), walk) - data.adjacent and type(round) == "number" and
                            round > data.adjacent and fetch ~= nil and fetch.kind == ACTION.MOVE and
                            goal.x == rogue.world.x and goal.y == rogue.world.y,
                            string.format(
                                "task=%s kind=%s dest=%s,%s steps=%s walk=%s walled=%s/%s %s,%s rogue=%s,%s round=%s",
                                tostring(pick and pick.task), tostring(pick and pick.kind), tostring(dest.x),
                                tostring(dest.y), tostring(dest.steps), tostring(walk), tostring(fetch and fetch.task),
                                tostring(fetch and fetch.kind), tostring(goal.x), tostring(goal.y),
                                tostring(rogue.world.x), tostring(rogue.world.y), tostring(round))
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
                    case("a crewmate beside me keeps the gap between casts still, and a mob beside me keeps a striker", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = apart_floors("poet65", "rogue50")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x - 1, snap.world.y
                        local near                                 = together.closer(snap, bound, combat.interval())
                        local warrior, guard                       = apart_floors("warrior63", "poet65")
                        local partner                              = warrior.party[1]
                        partner.world.map                          = warrior.world.map
                        partner.world.x                            = warrior.world.x + data.adjacent + 4
                        partner.world.y                            = warrior.world.y
                        warrior.world.mobs                         = { mob_named("칼든해골", warrior.world.x - 1,
                            warrior.world.y, 501) }
                        local fighting                             = together.closer(warrior, guard, combat.interval())
                        return near == nil and fighting == nil,
                            string.format("near=%s fighting=%s", tostring(near and near.kind),
                                tostring(fighting and fighting.kind))
                    end),
                    case("a poet the skeleton beside it comes for steps off it between casts", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 3, snap.world.y
                        local skeleton                             = mob_named("칼든해골", snap.world.x - 1, snap.world.y,
                            501)
                        snap.world.mobs                            = { skeleton }
                        local chaser                               = party.chaser(snap, skeleton, snap.world)
                        local step                                 = together.closer(snap, bound, combat.interval())
                        local dest                                 = step ~= nil and step.dest or {}
                        local off                                  = dest.x ~= nil and
                            math.abs(dest.x - skeleton.x) + math.abs(dest.y - skeleton.y) > 1
                        return chaser ~= nil and chaser.name == snap.self.name and step ~= nil and
                            step.kind == ACTION.MOVE and off,
                            string.format("chaser=%s kind=%s dest=%s,%s", tostring(chaser and chaser.name),
                                tostring(step and step.kind), tostring(dest.x), tostring(dest.y))
                    end),
                    case("a poet whose swing outdoes its spell still steps off the mob that comes for it", function()
                        local together                             = require("brain.trees.together")
                        local score                                = require("brain.policies.score")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 3, snap.world.y
                        local skeleton                             = mob_named("칼든해골", snap.world.x - 1, snap.world.y,
                            501)
                        snap.world.mobs                            = { skeleton }
                        local blow                                 = score.capability(snap, bound).blow
                        for _ = 1, 200 do
                            if blow.adjacent < blow.ranged then
                                snap.self.dam = (snap.self.dam or 0) + 10
                                blow          = score.capability(snap, bound).blow
                            end
                        end
                        local step = together.closer(snap, bound, combat.interval())
                        local dest = step ~= nil and step.dest or {}
                        local off  = dest.x ~= nil and math.abs(dest.x - skeleton.x) + math.abs(dest.y - skeleton.y) > 1
                        return blow.adjacent >= blow.ranged and blow.heal > blow.adjacent and step ~= nil and off,
                            string.format("adjacent=%.0f ranged=%.0f heal=%.0f kind=%s dest=%s,%s", blow.adjacent,
                                blow.ranged,
                                blow.heal or -1, tostring(step and step.kind), tostring(dest.x), tostring(dest.y))
                    end),
                    case("a mage alone steps off the mob that comes for it between casts", function()
                        local together    = require("brain.trees.together")
                        local crewed      = apart_floors("poet65", "warrior63")
                        local snap, bound = state("mage60")
                        snap.world.map    = crewed.world.map
                        local skeleton    = mob_named("칼든해골", snap.world.x - 1, snap.world.y, 501)
                        snap.world.mobs   = { skeleton }
                        local step        = together.closer(snap, bound, combat.interval())
                        local dest        = step ~= nil and step.dest or {}
                        local off         = dest.x ~= nil and
                            math.abs(dest.x - skeleton.x) + math.abs(dest.y - skeleton.y) > 1
                        return step ~= nil and step.kind == ACTION.MOVE and off,
                            string.format("kind=%s dest=%s,%s", tostring(step and step.kind), tostring(dest.x),
                                tostring(dest.y))
                    end),
                    case("a warrior alone stays beside the mob it swings at, and a mage alone with no mob about stays", function()
                        local together       = require("brain.trees.together")
                        local crewed         = apart_floors("poet65", "warrior63")
                        local warrior, guard = state("warrior63")
                        warrior.world.map    = crewed.world.map
                        warrior.world.mobs   = { mob_named("칼든해골", warrior.world.x - 1, warrior.world.y, 501) }
                        local fighting       = together.closer(warrior, guard, combat.interval())
                        local mage, bound    = state("mage60")
                        mage.world.map       = crewed.world.map
                        local idle           = together.closer(mage, bound, combat.interval())
                        return fighting == nil and idle == nil,
                            string.format("fighting=%s idle=%s", tostring(fighting and fighting.kind),
                                tostring(idle and idle.kind))
                    end),
                    case("within its crew's reach a poet steps off the mob on it though the step is a tile farther", function()
                        local spacing                              = require("brain.policies.spacing")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                        local skeleton                             = mob_named("칼든해골", snap.world.x - 1, snap.world.y,
                            501)
                        snap.world.mobs                            = { skeleton }
                        local place                                = spacing.place(snap, bound, 1)
                        local off                                  = place ~= nil and
                            math.abs(place.x - skeleton.x) + math.abs(place.y - skeleton.y) > 1
                        local farther                              = place ~= nil and
                            math.abs(place.x - mate.world.x) + math.abs(place.y - mate.world.y) > 1
                        return off and farther and place.spared > 0 and place.closer < 0,
                            string.format("place=%s spared=%s closer=%s", place and (place.x .. "," .. place.y) or "stay",
                                tostring(place and place.spared), tostring(place and place.closer))
                    end),
                    case("a poet whose warrior holds its wards casts them on itself under a skeleton", function()
                        local aid                                  = require("brain.tasks.aid")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                        mate.buffs                                 = { "무장", "보호" }
                        local kept                                 = {}
                        for _, name in ipairs(snap.buffs) do
                            if name ~= "무장" and name ~= "보호" then
                                table.insert(kept, name)
                            end
                        end
                        snap.buffs      = kept
                        snap.world.mobs = { mob_named("칼든해골", snap.world.x - 1, snap.world.y, 501) }
                        local action    = aid.offer("ward")(snap, bound)
                        return action ~= nil and action.kind == ACTION.CAST and action.oid == nil and
                            (action.key == "보호" or action.key == "무장") and action.ward ~= nil,
                            string.format("kind=%s key=%s oid=%s ward=%s", tostring(action and action.kind),
                                tostring(action and action.key), tostring(action and action.oid),
                                tostring(action and action.ward))
                    end),
                    case("within its crew's reach a poet's step off the mob on it is worth the blows it spares", function()
                        local spacing                              = require("brain.policies.spacing")
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                        snap.world.mobs                            = { mob_named("칼든해골", snap.world.x - 1,
                            snap.world.y, 501) }
                        local place                                = spacing.place(snap, bound, 1)
                        local worth                                = together.close_worth(snap, bound)
                        return place ~= nil and place.spared > 0 and place.closer < 0 and worth ~= nil and worth > 0,
                            string.format("place=%s spared=%s closer=%s worth=%s",
                                place and (place.x .. "," .. place.y) or "stay", tostring(place and place.spared),
                                tostring(place and place.closer), tostring(worth))
                    end),
                    case("past its crew's reach a poet closes in before it steps off a mob", function()
                        local spacing                              = require("brain.policies.spacing")
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        local reach                                = require("brain.policies.escort").reach()
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + reach + 3,
                            snap.world.y
                        local skeleton                             = mob_named("칼든해골", snap.world.x + 1, snap.world.y,
                            501)
                        snap.world.mobs                            = { skeleton }
                        local place                                = spacing.place(snap, bound, 1)
                        return place == nil or place.closer > 0,
                            string.format("place=%s spared=%s closer=%s", place and (place.x .. "," .. place.y) or "stay",
                                tostring(place and place.spared), tostring(place and place.closer))
                    end),
                    case("with its crew on another floor a poet takes no step off the mob on it", function()
                        local spacing     = require("brain.policies.spacing")
                        local snap, bound = apart_floors("poet65", "warrior63")
                        local skeleton    = mob_named("칼든해골", snap.world.x - 1, snap.world.y, 501)
                        snap.world.mobs   = { skeleton }
                        local place       = spacing.place(snap, bound, 1)
                        return place == nil, string.format("place=%s", place and (place.x .. "," .. place.y) or "stay")
                    end),
                    case("stairs four tiles off behind a wall are walked to before the crew crosses", function()
                        local snap, bound                                      = recorded("stairs_behind_wall")
                        snap.buffs                                             = buffed(snap)
                        snap.world.map, snap.world.x, snap.world.y             = 93, 3, 9
                        snap.world.mobs, snap.world.occupied, snap.world.items = {}, {}, {}
                        snap.record.memory.leg                                 = nil
                        snap.record.memory.battle                              = nil
                        snap.record.memory.blocked                             = nil
                        for index, member in ipairs(snap.party) do
                            member.world                                = { map = 93, x = 3, y = 9 + index }
                            member.heading, member.going, member.battle = nil, nil, nil
                        end
                        local exit     = bound.route({ 94 }).path[1]
                        local straight = math.abs(exit.x - snap.world.x) + math.abs(exit.y - snap.world.y)
                        local walk     = bound.steps(snap.world.map, snap.world.x, snap.world.y,
                            { { x = exit.x, y = exit.y, range = 0 } })[1]
                        data.trace     = true
                        local result   = decide.choose(snap, bound, nil, nil)
                        local rows     = decide.offers[snap.self.name] or {}
                        data.trace     = false
                        local path     = rows[1] ~= nil and rows[1].path or ""
                        local dest     = result ~= nil and result.dest or {}
                        return straight <= data.adjacent and walk > data.adjacent and
                            string.find(path, "next_floor", 1, true) ~= nil and dest.maps == nil and dest.x == exit.x and
                            dest.y == exit.y,
                            string.format("exit=%s,%s straight=%s walk=%s dest=%s,%s maps=%s steps=%s path=%s",
                                tostring(exit.x),
                                tostring(exit.y), tostring(straight), tostring(walk), tostring(dest.x), tostring(dest.y),
                                tostring(dest.maps and dest.maps[1]), tostring(dest.steps), path)
                    end),
                    case("an entrance four tiles off behind a wall is walked to before the member waits at it", function()
                        local snap, bound = recorded("member_waits_behind_door")
                        local door        = bound.route({ party.plan(snap, bound).floors[1].map }).path[1]
                        local straight    = math.abs(door.x - snap.world.x) + math.abs(door.y - snap.world.y)
                        local walk        = bound.steps(snap.world.map, snap.world.x, snap.world.y,
                            { { x = door.x, y = door.y, range = 0 } })[1]
                        data.trace        = true
                        local result      = decide.choose(snap, bound, nil, nil)
                        local rows        = decide.offers[snap.self.name] or {}
                        data.trace        = false
                        local path        = rows[1] ~= nil and rows[1].path or ""
                        local dest        = result ~= nil and result.dest or {}
                        return straight <= data.adjacent and walk > data.adjacent and
                            string.find(path, "follow_entry", 1, true) ~= nil and result.kind == ACTION.MOVE and
                            dest.x == door.x and dest.y == door.y,
                            string.format("door=%s,%s straight=%s walk=%s kind=%s dest=%s,%s steps=%s path=%s",
                                tostring(door.x), tostring(door.y), tostring(straight), tostring(walk),
                                tostring(result and result.kind), tostring(dest.x), tostring(dest.y),
                                tostring(dest.steps),
                                path)
                    end),
                    case("a step off the mobs is not taken through a crewmate's tile", function()
                        local spacing     = require("brain.policies.spacing")
                        local snap, bound = recorded("poet_steps_through_crewmate")
                        local mate        = snap.party[1].world
                        local place       = spacing.place(snap, bound, 2)
                        local routes      = place ~= nil and
                            select(2, bound.steps(snap.world.map, snap.world.x, snap.world.y,
                                { { x = place.x, y = place.y, range = 0 } }, true)) or {}
                        local way         = {}
                        local through     = false
                        for _, at in ipairs(routes[1] or {}) do
                            table.insert(way, at.x .. "," .. at.y)
                            through = through or (at.x == mate.x and at.y == mate.y)
                        end
                        return through == false, string.format("place=%s mate=%s,%s way=%s",
                            place and (place.x .. "," .. place.y) or "stay", tostring(mate.x), tostring(mate.y),
                            table.concat(way, ">"))
                    end),
                    case("making way I do not take the tile my crewmate walks to", function()
                        local snap, bound = recorded("striker_takes_goal_tile")
                        local going       = snap.party[1].going or {}
                        local pick        = decide.choose(snap, bound, nil, nil)
                        local dest        = pick ~= nil and pick.kind == ACTION.MOVE and pick.dest or {}
                        return going.x ~= nil and (dest.x ~= going.x or dest.y ~= going.y),
                            string.format("going=%s,%s dest=%s,%s", tostring(going.x), tostring(going.y),
                                tostring(dest.x), tostring(dest.y))
                    end),
                    case("the tile a crewmate's walk stopped at is no place of mine", function()
                        local spacing = require("brain.policies.spacing")
                        local function pick(stopped)
                            local snap, bound                          = apart_floors("poet65", "warrior63")
                            local mate                                 = snap.party[1]
                            mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                            mate.blocker                               = stopped
                            snap.world.mobs                            = { mob_named("칼든해골", snap.world.x - 1,
                                snap.world.y, 501) }
                            return spacing.place(snap, bound, 1)
                        end
                        local free = pick(nil)
                        local kept = free ~= nil and pick({ x = free.x, y = free.y }) or nil
                        return free ~= nil and (kept == nil or kept.x ~= free.x or kept.y ~= free.y),
                            string.format("free=%s kept=%s", free and (free.x .. "," .. free.y) or "stay",
                                kept and (kept.x .. "," .. kept.y) or "stay")
                    end),
                    case("a poet does not step onto its rogue's only way out of a dead end", function()
                        local spacing                              = require("brain.policies.spacing")
                        local snap, bound                          = apart_floors("poet65", "rogue50")
                        local mate                                 = snap.party[1]
                        snap.world.map, snap.world.x, snap.world.y = 91, 28, 23
                        mate.world.map, mate.world.x, mate.world.y = 91, 29, 20
                        snap.world.mobs                            = {}
                        local place                                = spacing.place(snap, bound, 1)
                        local shut                                 = bound.detour(91, mate.world, snap.world,
                            { x = 29, y = 23 })
                        return shut == math.huge and (place == nil or place.x ~= 29 or place.y ~= 23),
                            string.format("place=%s shut=%s", place and (place.x .. "," .. place.y) or "stay",
                                tostring(shut))
                    end),
                    case("a walk stopped at a crewmate keeps its goal in my profile", function()
                        local snap, bound               = apart_floors("warrior63", "poet65")
                        local x, y                      = snap.world.x, snap.world.y
                        snap.record.memory.spot         = { map = snap.world.map, x = x, y = y, at = snap.now - 1000 }
                        snap.record.memory.blocked      = {
                            at = snap.now - 500,
                            since = snap.now - 500,
                            map = snap.world.map,
                            detail = "CHARACTER",
                            by = { x = x, y = y + 1 },
                            dest = { map = snap.world.map, x = x, y = y + 6, range = 1 }
                        }
                        local posted                    = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.HOLD })
                        local going                     = posted.going or {}
                        snap.record.memory.blocked.dest = { map = snap.world.map, x = x, y = y + 1, range = 0 }
                        local at_goal                   = party.profile(snap, bound,
                            { task = "FIELD", kind = ACTION.HOLD })
                        return posted.blocker ~= nil and going.x == x and going.y == y + 6 and at_goal.blocker ~= nil and
                            at_goal.going == nil,
                            string.format("blocker=%s going=%s,%s at_goal=%s", tostring(posted.blocker ~= nil),
                                tostring(going.x), tostring(going.y), tostring(at_goal.going ~= nil))
                    end),
                    case("a low warrior whose spell outdoes one swing still fights from a mob's side, and casters do not", function()
                        local score = require("brain.policies.score")
                        local seen  = {}
                        for _, name in ipairs({ "warrior10", "warrior63", "rogue50", "mage60", "poet65" }) do
                            local snap, bound = state(name)
                            local blow        = score.capability(snap, bound).blow
                            seen[name]        = score.melee(snap, bound)
                            seen[name .. "!"] = string.format("%s:%.0f/%.0f", tostring(seen[name]), blow.adjacent,
                                blow.ranged)
                        end
                        return seen.warrior10 and seen.warrior63 and seen.rogue50 and seen.mage60 == false and
                            seen.poet65 == false,
                            table.concat(
                                { seen["warrior10!"], seen["warrior63!"], seen["rogue50!"], seen["mage60!"], seen
                                    ["poet65!"] },
                                " ")
                    end),
                    case("a mob comes for the crewmate that hurt it most, else for the nearest", function()
                        local function chased(dealt)
                            local snap, _                              = apart_floors("poet65", "warrior63")
                            local mate                                 = snap.party[1]
                            mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 3, snap.world.y
                            mate.dealt                                 = dealt
                            local skeleton                             = mob_named("칼든해골", snap.world.x - 1, snap.world
                                .y, 501)
                            snap.world.mobs                            = { skeleton }
                            local found                                = party.chaser(snap, skeleton, snap.world)
                            return found ~= nil and
                                (found.name == snap.self.name and "me" or found.name == mate.self.name and "mate") or
                                "none"
                        end
                        local near = chased(nil)
                        local hurt = chased({ ["501"] = { damage = 500, at = 1000, since = 0 } })
                        return near == "me" and hurt == "mate", string.format("near=%s hurt=%s", near, hurt)
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
                    case("a healer closing in never steps on a warp tile", function()
                        local spacing = require("brain.policies.spacing")
                        local function placed(query)
                            local snap, bound                                                     = apart_floors(
                                "poet65", "warrior63")
                            local rogue                                                           = state("rogue50")
                            rogue.world.map                                                       = snap.world.map
                            snap.party[1].world.map, snap.party[1].world.x, snap.party[1].world.y = snap.world.map, 4, 10
                            rogue.world.x, rogue.world.y                                          = 16, 10
                            table.insert(snap.party, party.profile(rogue, bound, nil))
                            table.insert(snap.group, rogue.self.name)
                            snap.world.x, snap.world.y = 22, 10
                            return spacing.place(snap, query ~= nil and setmetatable(query, { __index = bound }) or bound,
                                2)
                        end
                        local free   = placed(nil)
                        local warped = free ~= nil and placed({
                            warp = function(_, x, y)
                                return x == free.x and y == free.y
                            end,
                        }) or nil
                        return free ~= nil and (warped == nil or warped.x ~= free.x or warped.y ~= free.y),
                            string.format("free=%s warped=%s", free and (free.x .. "," .. free.y) or "stay",
                                warped and (warped.x .. "," .. warped.y) or "stay")
                    end),
                    case("the vanguard's walk toward a crewmate at the edge of walking reach is not shortened", function()
                        local escort                               = require("brain.policies.escort")
                        local snap, bound                          = apart_floors("warrior63", "poet65")
                        local mate                                 = snap.party[1]
                        snap.world.map, snap.world.x, snap.world.y = 421, 18, 6
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, 14, 9
                        mate.task                                  = "FIELD"
                        local plan                                 = party.plan(snap, bound)
                        local name                                 = nil
                        for _, floor in ipairs(plan.floors) do
                            if floor.map == snap.world.map then
                                name = floor.targets[1]
                            end
                        end
                        snap.world.mobs = { mob_named(name, 15, 8, 777) }
                        local walk      = bound.steps(snap.world.map, 14, 9, { { x = 18, y = 6, range = 0 } })[1]
                        local pick      = decide.choose(snap, bound, nil, nil)
                        local dest      = pick ~= nil and pick.dest or {}
                        return walk == escort.reach() and pick ~= nil and pick.kind == ACTION.MOVE and
                            dest.x == 15 and dest.y == 8 and (dest.steps or 0) == 0,
                            string.format("walk=%s kind=%s task=%s dest=%s,%s steps=%s", tostring(walk),
                                tostring(pick and pick.kind), tostring(pick and pick.task), tostring(dest.x),
                                tostring(dest.y), tostring(dest.steps))
                    end),
                    case("a poet on a pile picks it up before it closes on its vanguard that walked off", function()
                        local snap, bound                          = apart_floors("poet65", "warrior63")
                        local mate                                 = snap.party[1]
                        snap.world.map, snap.world.x, snap.world.y = 421, 18, 6
                        mate.world.map, mate.world.x, mate.world.y = snap.world.map, 14, 9
                        mate.task                                  = "FIELD"
                        snap.world.mobs                            = {}
                        snap.world.items                           = { { oid = 951, x = 18, y = 6, names = { "여우모피" } } }
                        local close                                = require("brain.trees.together").close(snap, bound)
                        local pick                                 = decide.choose(snap, bound, nil, nil)
                        return close ~= nil and pick ~= nil and pick.kind == ACTION.PICK,
                            string.format("close=%s pick=%s/%s", tostring(close and close.kind),
                                tostring(pick and pick.task),
                                tostring(pick and pick.kind))
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
                            detour = function(_, from, to, avoided)
                                local between = (avoided.x - from.x) * (avoided.x - to.x) <= 0
                                return avoided.y == row and between and math.huge or 0
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
                    case("a member on the way to the stairs makes way for a crewmate walking to the next map", function()
                        local together = require("brain.trees.together")
                        local function made(goal)
                            local snap, bound                          = state("poet94")
                            local mate                                 = state("warrior93")
                            snap.world.map, snap.world.x, snap.world.y = 22, 31, 29
                            mate.world.map, mate.world.x, mate.world.y = 22, 31, 28
                            mate.record.memory.spot                    = { map = 22, at = mate.now - 1000 }
                            mate.record.memory.blocked                 = {
                                detail = "CHARACTER",
                                at = mate.now,
                                since = mate.now,
                                map = 22,
                                dest = { maps = { 2376 } },
                                by = { x = 31, y = 29 },
                                goal = goal,
                            }
                            mate.group                                 = { mate.self.name, snap.self.name }
                            snap.group                                 = mate.group
                            snap.leader                                = mate.self.name
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "FIELD"
                            local found                                = require("brain.tree").run(together.make_way,
                                snap, bound)
                            return found ~= nil and found.action ~= nil and found.action.dest or {}
                        end
                        local told  = made({ map = 22, x = 31, y = 30 })
                        local blind = made(nil)
                        local onto  = told.x ~= nil and told.y == 30 and (told.x == 30 or told.x == 31)
                        return told.x ~= nil and onto == false and blind.x == nil,
                            string.format("told=%s,%s blind=%s,%s", tostring(told.x), tostring(told.y),
                                tostring(blind.x), tostring(blind.y))
                    end),
                    case("a member making way does not step onto the tile a mob stands on", function()
                        local together = require("brain.trees.together")
                        local function aside(mobbed)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 964, 3, 1
                            mate.world.map, mate.world.x, mate.world.y = 964, 2, 1
                            snap.group                                 = { mate.self.name, snap.self.name }
                            snap.leader                                = mate.self.name
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "FIELD"
                            snap.party[1].going                        = { map = 964, x = 8, y = 5 }
                            snap.world.mobs                            = mobbed ~= nil and
                                { mob_named("해골", mobbed.x, mobbed.y, 771) } or {}
                            local found                                = require("brain.tree").run(together.make_way,
                                snap, bound)
                            return found ~= nil and found.action ~= nil and found.action.dest or {}
                        end
                        local free  = aside(nil)
                        local taken = aside(free.x ~= nil and { x = free.x, y = free.y } or nil)
                        return free.x ~= nil and (taken.x ~= free.x or taken.y ~= free.y),
                            string.format("free=%s,%s taken=%s,%s", tostring(free.x), tostring(free.y), tostring(taken.x),
                                tostring(taken.y))
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
                    case("a member on open floor does not step aside for a crewmate that can walk round it", function()
                        local together                             = require("brain.trees.together")
                        local snap, bound                          = state("poet65")
                        local mate                                 = state("rogue50")
                        snap.world.map, snap.world.x, snap.world.y = 92, 17, 11
                        mate.world.map, mate.world.x, mate.world.y = 92, 16, 11
                        snap.group                                 = { mate.self.name, snap.self.name }
                        snap.leader                                = mate.self.name
                        snap.party                                 = { party.profile(mate, bound, nil) }
                        snap.party[1].task                         = "FIELD"
                        snap.party[1].going                        = { map = 92, x = 27, y = 19 }
                        local found                                = require("brain.tree").run(together.make_way, snap,
                            bound)
                        local dest                                 = found ~= nil and found.action ~= nil and
                            found.action.dest or nil
                        return dest == nil,
                            string.format("dest=%s,%s", tostring(dest and dest.x), tostring(dest and dest.y))
                    end),
                    case("a member its crewmate's walk stopped at makes way though a walk round it looks short", function()
                        local together = require("brain.trees.together")
                        local going    = { map = 5246, x = 16, y = 8 }
                        local function made(stopped)
                            local snap, bound                          = state("poet65")
                            local mate                                 = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y = 5246, 21, 9
                            mate.world.map, mate.world.x, mate.world.y = 5246, 21, 8
                            snap.group                                 = { mate.self.name, snap.self.name }
                            snap.leader                                = mate.self.name
                            mate.group, mate.leader                    = snap.group, snap.leader
                            mate.record.memory.spot                    = {
                                map = 5246,
                                x = 21,
                                y = 8,
                                at = mate.now -
                                    2000
                            }
                            mate.record.memory.blocked                 = stopped and {
                                map = 5246,
                                at = mate.now,
                                since = mate.now,
                                detail = "CHARACTER",
                                dest = going,
                                by = { x = 21, y = 9 }
                            } or nil
                            snap.party                                 = { party.profile(mate, bound, nil) }
                            snap.party[1].task                         = "FIELD"
                            snap.party[1].going                        = going
                            local found                                = require("brain.tree").run(together.make_way,
                                snap, bound)
                            local dest                                 = found ~= nil and found.action ~= nil and
                                found.action.dest or nil
                            return dest, dest ~= nil and bound.detour(5246, mate.world, going, dest) or nil,
                                bound.detour(5246, mate.world, going, snap.world)
                        end
                        local free               = made(false)
                        local dest, after, round = made(true)
                        return free == nil and dest ~= nil and round > 0 and after == 0,
                            string.format("free=%s dest=%s,%s round=%s after=%s", tostring(free ~= nil),
                                tostring(dest and dest.x), tostring(dest and dest.y), tostring(round), tostring(after))
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
                        return walking.pinned == true and drinking.pinned == true,
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
                    case("a warrior keeps 건곤대나이's mana for it instead of casting 백열장, healed, alone or under 운기", function()
                        local value = require("brain.policies.value")
                        local function pick(grouped, surging)
                            local snap, bound = state("warrior63")
                            local mob         = mob_named("칼든해골", snap.world.x + 1, snap.world.y, 801)
                            snap.world.mobs   = { mob }
                            snap.self.hp      = snap.self.max_hp // 3
                            snap.self.mp      = snap.self.max_mp // 2
                            if surging then
                                local known = false
                                for _, learned in ipairs(snap.spells) do
                                    known = known or learned.name == "운기"
                                end
                                if known == false then
                                    table.insert(snap.spells, { name = "운기", slot = 99 })
                                end
                            end
                            if grouped then
                                local poet                                 = state("poet65")
                                poet.world.map, poet.world.x, poet.world.y = snap.world.map, snap.world.x - 1,
                                    snap.world.y
                                snap.group                                 = { snap.self.name, poet.self.name }
                                snap.leader                                = snap.self.name
                                snap.party                                 = { party.profile(poet, bound, nil) }
                            end
                            local choice = battle.choose(snap, bound, mob, true)
                            return choice and choice.key, value.held(snap, bound, mob)
                        end
                        local alone, kept    = pick(false)
                        local beside, held   = pick(true)
                        local surged, banked = pick(false, true)
                        return beside ~= "백열장" and held > 0 and alone ~= "백열장" and kept > 0 and surged ~= "백열장" and
                            banked > 0,
                            string.format("alone=%s kept=%.1f beside=%s held=%.1f surged=%s banked=%.1f", tostring(alone),
                                kept, tostring(beside), held, tostring(surged), banked)
                    end),
                    case("a poet beside its warrior takes no prey it must walk to, only one beside it", function()
                        local function prey(gap)
                            local snap, bound                                   = state("poet65")
                            local warrior                                       = state("warrior63")
                            snap.world.map, snap.world.x, snap.world.y          = 420, 25, 26
                            warrior.world.map, warrior.world.x, warrior.world.y = 420, 24, 26
                            snap.world.mobs                                     = { mob_named("해골", 25 + gap, 26, 903) }
                            snap.world.mobs[1].applied                          = { ["혼마술"] = true }
                            snap.record.memory.marks                            = { ["903"] = { ["혼마술"] = snap.now } }
                            snap.party                                          = { party.profile(warrior, bound, nil) }
                            snap.group                                          = { warrior.self.name, snap.self.name }
                            snap.leader                                         = warrior.self.name
                            if party.vanguard_name(snap, bound) ~= warrior.self.name then
                                return false, "premise: the warrior is not the vanguard"
                            end
                            local found = require("brain.tasks.field").target(snap, bound, { "해골" })
                            return found ~= nil, found ~= nil and found.choice.key or nil
                        end
                        local far, far_key   = prey(4)
                        local near, near_key = prey(1)
                        return far == false and near == true,
                            string.format("far=%s/%s near=%s/%s", tostring(far), tostring(far_key), tostring(near),
                                tostring(near_key))
                    end),
                    case("a warrior struck beside its poet draws the poet's attacker once sent to save it", function()
                        local function strike(sent)
                            local snap, bound                          = state("warrior63")
                            local poet                                 = state("poet65")
                            snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                            poet.world.map, poet.world.x, poet.world.y = 420, 26, 26
                            snap.self.struck                           = 0
                            snap.world.mobs                            = { mob_named("해골", 24, 26, 901),
                                mob_named("해골", 27, 26, 902) }
                            snap.party                                 = { party.profile(poet, bound, nil) }
                            snap.group                                 = { poet.self.name, snap.self.name }
                            snap.leader                                = poet.self.name
                            snap.party[1].rescue                       = sent and
                                { name = poet.self.name, by = snap.self.name, value = 5000000, shares = { ["902"] = 1 } } or
                                nil
                            local action                               = require("brain.tasks.defend").strike_back(snap,
                                bound)
                            return action ~= nil and action.oid, action ~= nil and action.key
                        end
                        local own, own_key     = strike(false)
                        local drawn, drawn_key = strike(true)
                        return own == 901 and drawn == 902,
                            string.format("unsent=%s/%s sent=%s/%s", tostring(own), tostring(own_key), tostring(drawn),
                                tostring(drawn_key))
                    end),
                    case("a warrior its own four mobs surround still draws its poet's attacker with a spell from afar", function()
                        local snap, bound                          = state("warrior63")
                        local poet                                 = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        poet.world.map, poet.world.x, poet.world.y = 420, 28, 26
                        snap.world.mobs                            = { mob_named("해골", 24, 26, 711), mob_named("해골", 25,
                            25, 712),
                            mob_named("해골", 26, 26, 713), mob_named("해골", 25, 27, 714), mob_named("칼든해골", 29, 26, 715) }
                        poet.world.mobs                            = { mob_named("칼든해골", 29, 26, 715) }
                        snap.self.struck, poet.self.struck         = 0, 0
                        snap.group                                 = { snap.self.name, poet.self.name }
                        poet.group                                 = snap.group
                        snap.leader                                = snap.self.name
                        snap.party                                 = { party.profile(poet, bound, nil) }
                        local rescue                               = party.rescue(snap, bound)
                        local action                               = require("brain.tasks.defend").strike_back(snap,
                            bound)
                        local open                                 = bound.floor(420, 28, 26) and
                            bound.floor(420, 29, 26)
                        return open and poet.self.hp == poet.self.max_hp and rescue ~= nil and
                            rescue.name == poet.self.name and
                            action ~= nil and action.oid == 715 and action.key == "백열장",
                            string.format("open=%s rescue=%s action=%s/%s", tostring(open),
                                tostring(rescue and rescue.name),
                                tostring(action and action.oid), tostring(action and action.key))
                    end),
                    case("a warrior its own four mobs surround draws one of the three mobs on its poet before its own", function()
                        local snap, bound                          = state("warrior63")
                        local poet                                 = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        poet.world.map, poet.world.x, poet.world.y = 420, 28, 26
                        local attackers                            = { mob_named("칼든해골", 29, 26, 715),
                            mob_named("칼든해골", 28, 25, 716), mob_named("칼든해골", 28, 27, 717) }
                        snap.world.mobs                            = { mob_named("해골", 24, 26, 711), mob_named("해골", 25,
                            25, 712),
                            mob_named("해골", 26, 26, 713), mob_named("해골", 25, 27, 714) }
                        poet.world.mobs                            = {}
                        for _, mob in ipairs(attackers) do
                            table.insert(snap.world.mobs, mob)
                            table.insert(poet.world.mobs, mob)
                        end
                        snap.self.mp                       = snap.self.max_mp
                        snap.self.struck, poet.self.struck = 0, 0
                        snap.group                         = { snap.self.name, poet.self.name }
                        poet.group                         = snap.group
                        snap.leader                        = snap.self.name
                        snap.party                         = { party.profile(poet, bound, nil) }
                        local rescue                       = party.rescue(snap, bound)
                        local action                       = require("brain.tasks.defend").strike_back(snap, bound)
                        local open                         = bound.floor(420, 29, 26) and bound.floor(420, 28, 25) and
                            bound.floor(420, 28, 27)
                        local drawn                        = action ~= nil and action.oid ~= nil and action.oid >= 715
                        return open and rescue ~= nil and rescue.name == poet.self.name and drawn and action.key == "백열장",
                            string.format("open=%s rescue=%s action=%s/%s", tostring(open),
                                tostring(rescue and rescue.name),
                                tostring(action and action.oid), tostring(action and action.key))
                    end),
                    case("a crewmate's peril counts the damage it has dealt its attackers, not the spells it could cast", function()
                        local function peril(dealt)
                            local snap, bound         = state("poet65")
                            local skeleton            = mob_named("해골", snap.world.x + 1, snap.world.y, 781)
                            snap.world.mobs           = { skeleton }
                            snap.self.struck          = 0
                            snap.record.memory.battle = { oid = 781, map = snap.world.map }
                            snap.record.memory.dealt  = dealt ~= nil and
                                { ["781"] = { damage = dealt.damage, at = dealt.at, since = dealt.since, map = snap.world.map } } or
                                nil
                            return party.peril(snap, bound)
                        end
                        local swung = peril({ damage = 330, at = 1000, since = 1000 - 3000 })
                        local none  = peril(nil)
                        return swung ~= nil and math.abs(swung.dps - 110) < 1e-6 and none ~= nil and none.dps == 0,
                            string.format("swung=%s none=%s", tostring(swung and swung.dps), tostring(none and none.dps))
                    end),
                    case("a walk round mobs standing in the way counts the way round, and the bare walk when none is left", function()
                        local snap, bound = state("poet65")
                        local map         = snap.world.map
                        local cx, cy      = open_ground(bound, map)
                        if cx == nil then
                            return false, "no open ground on map " .. tostring(map)
                        end
                        local goal                 = { { x = cx + 3, y = cy, range = 0 } }
                        snap.world.x, snap.world.y = cx, cy
                        snap.world.occupied        = {}
                        local bare                 = bound.steps(map, cx, cy, goal)[1]
                        snap.world.occupied        = { { x = cx + 1, y = cy - 1 }, { x = cx + 1, y = cy }, { x = cx + 1, y = cy + 1 },
                            { x = cx + 3, y = cy } }
                        local round                = bound.steps(map, cx, cy, goal)[1]
                        snap.world.occupied        = { { x = cx + 1, y = cy }, { x = cx - 1, y = cy }, { x = cx, y = cy - 1 },
                            { x = cx, y = cy + 1 } }
                        local sealed               = bound.steps(map, cx, cy, goal)[1]
                        return bare == 3 and type(round) == "number" and round > bare and sealed == bare,
                            string.format("map=%d at=%d,%d bare=%s round=%s sealed=%s", map, cx, cy, tostring(bare),
                                tostring(round), tostring(sealed))
                    end),
                    case("a crewmate's walk to me does not go round the mobs I see about", function()
                        local escort      = require("brain.policies.escort")
                        local snap, bound = apart_floors("poet65", "warrior63")
                        local map         = snap.world.map
                        local cx, cy      = open_ground(bound, map)
                        if cx == nil then
                            return false, "no open ground on map " .. tostring(map)
                        end
                        local mate                                 = snap.party[1]
                        snap.world.x, snap.world.y                 = cx, cy
                        mate.world.map, mate.world.x, mate.world.y = map, cx + 3, cy
                        snap.world.occupied                        = { { x = cx + 2, y = cy - 1 }, { x = cx + 2, y = cy },
                            { x = cx + 2, y = cy + 1 } }
                        local member                               = party.members(snap)[1]
                        local theirs                               = escort.steps(snap, bound, member,
                            { { x = cx, y = cy } })[1]
                        local mine                                 = escort.steps(snap, bound, snap,
                            { { x = cx + 3, y = cy } })[1]
                        return theirs == 3 and type(mine) == "number" and mine > 3,
                            string.format("theirs=%s mine=%s", tostring(theirs), tostring(mine))
                    end),
                    case("a heal out of a fight is worth what it outweighs the strike at my prey it gives up", function()
                        local aid = require("brain.tasks.aid")
                        local function weighed(prey, strong)
                            local snap, bound = apart_floors("poet65", "warrior63")
                            local mate        = snap.party[1]
                            if strong == false then
                                local kept = {}
                                for _, learned in ipairs(mate.spells) do
                                    if learned.name ~= "건곤대나이" then
                                        table.insert(kept, learned)
                                    end
                                end
                                mate.spells = kept
                            end
                            without(snap, "혼마술")
                            local floor                                = party.plan(snap, bound).floors[1]
                            local names                                = floor.targets
                            snap.world.map                             = floor.map
                            mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 1, snap.world.y
                            mate.self.hp                               = math.floor(mate.self.max_hp * 0.3)
                            bound.ally                                 = function(name)
                                return name == mate.self.name and { oid = 9001, percent = 30 } or nil
                            end
                            if prey then
                                snap.world.mobs = { mob_named(names[1], snap.world.x - 2, snap.world.y, 761) }
                            end
                            return aid.worth("heal")(snap, bound), aid.spared("heal")(snap, bound),
                                aid.forgone(snap, bound)
                        end
                        local heal, alone, none  = weighed(false, false)
                        local _, beside, forgone = weighed(true, false)
                        local _, fueled, held    = weighed(true, true)
                        return heal ~= nil and alone == heal and none == 0 and forgone > 0 and beside ~= nil and
                            math.abs(beside - (heal - forgone)) < 1e-6 and held < forgone,
                            string.format("heal=%s alone=%s none=%s beside=%s forgone=%s fueled=%s held=%s",
                                tostring(heal),
                                tostring(alone), tostring(none), tostring(beside), tostring(forgone), tostring(fueled),
                                tostring(held))
                    end),
                    case("a mob beside me that a crewmate hurt most is no attacker of mine", function()
                        local function perilous(dealt)
                            local snap, bound                          = apart_floors("poet65", "warrior63")
                            local mate                                 = snap.party[1]
                            mate.world.map, mate.world.x, mate.world.y = snap.world.map, snap.world.x + 2, snap.world.y
                            mate.dealt                                 = dealt
                            snap.world.mobs                            = { mob_named("칼든해골", snap.world.x + 1,
                                snap.world.y, 781) }
                            snap.self.struck                           = 0
                            return party.peril(snap, bound)
                        end
                        local mine   = perilous(nil)
                        local theirs = perilous({ ["781"] = { damage = 900, at = 1000, since = 0 } })
                        return mine ~= nil and #mine.attackers == 1 and theirs == nil,
                            string.format("mine=%s theirs=%s", tostring(mine and #mine.attackers),
                                tostring(theirs and #theirs.attackers))
                    end),
                    case("a poet its warrior is sent to draw from afar does not swing at the skeleton on it", function()
                        local snap, bound = recorded("poet_swings_awaiting_draw")
                        local rescue      = party.awaited(snap, bound)
                        local pick        = decide.choose(snap, bound, nil, nil)
                        local held        = pick ~= nil and pick.kind == ACTION.ATTACK and pick.oid ~= nil and
                            rescue ~= nil and
                            (rescue.shares or {})[tostring(pick.oid)] ~= nil and (pick.damage or 0) > 0
                        return rescue ~= nil and rescue.walks == false and held == false,
                            string.format("rescue=%s walks=%s pick=%s/%s/%s/%s", tostring(rescue and rescue.by),
                                tostring(rescue and rescue.walks), tostring(pick and pick.task),
                                tostring(pick and pick.kind),
                                tostring(pick and pick.key), tostring(pick and pick.oid))
                    end),
                    case("a warrior's hp a poet heals costs the warrior's cast of its own heal as well as its mana", function()
                        local value       = require("brain.policies.value")
                        local recovery    = require("brain.policies.recovery")
                        local poet, bound = state("poet65")
                        local warrior     = state("warrior63")
                        without(warrior, "건곤대나이")
                        local worth = value.baseline(poet, bound)
                        local mana  = value.spend(poet, bound, nil)
                        local mend  = recovery.heal(warrior)
                        local alone = mend.mp / mend.hp * mana
                        local cast  = combat.cast(warrior) / 1000 * worth.value / mend.hp
                        local food  = value.food(warrior, bound, worth)
                        local price = value.vigor(warrior, bound, worth, mana)
                        return price > alone and math.abs(price - math.min(food, alone + cast)) < 1e-6,
                            string.format("price=%s mana_only=%s cast=%s food=%s", tostring(price), tostring(alone),
                                tostring(cast), tostring(food))
                    end),
                    case("a rogue with the mana casts 백열장 at the mob on its poet three tiles off", function()
                        local snap, bound                          = state("rogue50")
                        local poet                                 = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        poet.world.map, poet.world.x, poet.world.y = 420, 28, 26
                        local skeleton                             = mob_named("칼든해골", 29, 26, 751)
                        snap.world.mobs, poet.world.mobs           = { skeleton }, { skeleton }
                        snap.self.mp                               = 330
                        snap.self.struck, poet.self.struck         = nil, 0
                        snap.group                                 = { snap.self.name, poet.self.name }
                        poet.group                                 = snap.group
                        snap.leader                                = snap.self.name
                        snap.party                                 = { party.profile(poet, bound, nil) }
                        local rescue                               = party.rescue(snap, bound)
                        local action                               = require("brain.tasks.defend").strike_back(snap,
                            bound)
                        return rescue ~= nil and rescue.by == snap.self.name and action ~= nil and action.oid == 751 and
                            action.key == "백열장" and action.maneuver.kind == MANEUVER.CAST,
                            string.format("rescue=%s action=%s/%s", tostring(rescue and rescue.by),
                                tostring(action and action.oid),
                                tostring(action and action.key))
                    end),
                    case("a rogue short of mana drinks the least dose that makes up 백열장 at the mob on its poet", function()
                        local snap, bound                          = state("rogue50")
                        local poet                                 = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        poet.world.map, poet.world.x, poet.world.y = 420, 28, 26
                        local skeleton                             = mob_named("칼든해골", 29, 26, 752)
                        snap.world.mobs, poet.world.mobs           = { skeleton }, { skeleton }
                        snap.self.mp                               = 30
                        snap.bag                                   = {
                            items = { { name = "동동주", count = 100, slot = 1 }, { name = "홍주", count = 100, slot = 2 } },
                            used = 2,
                            capacity = snap.bag.capacity,
                        }
                        snap.self.struck, poet.self.struck         = nil, 0
                        snap.group                                 = { snap.self.name, poet.self.name }
                        poet.group                                 = snap.group
                        snap.leader                                = snap.self.name
                        snap.party                                 = { party.profile(poet, bound, nil) }
                        local rescue                               = party.rescue(snap, bound)
                        local action                               = require("brain.tasks.defend").strike_back(snap,
                            bound)
                        return rescue ~= nil and rescue.by == snap.self.name and rescue.walks == false and action ~= nil and
                            action.oid == 752 and action.maneuver.kind == MANEUVER.USE and action.maneuver.item == "홍주" and
                            (action.damage or 0) == 0,
                            string.format("rescue=%s walks=%s action=%s/%s/%s", tostring(rescue and rescue.by),
                                tostring(rescue and rescue.walks), tostring(action and action.oid),
                                tostring(action and action.key),
                                tostring(action and action.maneuver.item))
                    end),
                    case("a rogue's 투명 is no draw of the mob on its poet three tiles off, which it strikes nothing from there", function()
                        local snap, bound                          = state("rogue50")
                        local poet                                 = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 420, 25, 26
                        poet.world.map, poet.world.x, poet.world.y = 420, 28, 26
                        local skeleton                             = mob_named("칼든해골", 29, 26, 741)
                        snap.world.mobs, poet.world.mobs           = { skeleton }, { skeleton }
                        snap.self.mp                               = 0
                        snap.self.struck, poet.self.struck         = nil, 0
                        snap.group                                 = { snap.self.name, poet.self.name }
                        poet.group                                 = snap.group
                        snap.leader                                = snap.self.name
                        snap.party                                 = { party.profile(poet, bound, nil) }
                        local rescue                               = party.rescue(snap, bound)
                        local guard                                = party.rescue_share(snap, bound, skeleton)
                        local traced                               = data.trace
                        data.trace                                 = true
                        local choice                               = battle.choose(snap, bound, skeleton)
                        data.trace                                 = traced
                        local cloak, swing                         = nil, nil
                        for _, entry in ipairs(choice ~= nil and choice.rivals or {}) do
                            if entry.key == "투명" then
                                cloak = entry.value
                            elseif entry.key == "SWING" then
                                swing = entry.value
                            end
                        end
                        return rescue ~= nil and rescue.by == snap.self.name and guard > 0 and cloak ~= nil and
                            swing ~= nil and
                            cloak - swing < guard / 2,
                            string.format("rescue=%s/%s guard=%.0f cloak=%s swing=%s", tostring(rescue and rescue.name),
                                tostring(rescue and rescue.by), guard, tostring(cloak), tostring(swing))
                    end),
                    case("a poet struck beside the warrior its heals feed keeps its mana from 화염주, and spends it alone", function()
                        local function pick(crewed)
                            local snap, bound         = state("poet65")
                            local mob                 = mob_named("칼든해골", snap.world.x + 1, snap.world.y, 811)
                            snap.world.mobs           = { mob }
                            snap.self.mp              = snap.self.max_mp
                            snap.record.memory.battle = {
                                oid = mob.oid, map = snap.world.map, applied = { ["혼마술"] = snap.now }
                            }
                            if crewed then
                                local warrior                                       = state("warrior63")
                                warrior.world.map, warrior.world.x, warrior.world.y = snap.world.map, snap.world.x - 1,
                                    snap.world.y
                                snap.group                                          = { snap.self.name, warrior.self
                                    .name }
                                snap.leader                                         = warrior.self.name
                                snap.party                                          = { party.profile(warrior, bound, nil) }
                            end
                            local choice = battle.choose(snap, bound, mob)
                            return choice and choice.key
                        end
                        local alone, beside = pick(false), pick(true)
                        return alone == "화염주" and beside ~= "화염주",
                            string.format("alone=%s beside=%s", tostring(alone), tostring(beside))
                    end),
                    case("a side swing splashes the mobs at my sides, as the server swings, not the ones round the mob", function()
                        local snap, bound = state("rogue50")
                        local x, y        = snap.world.x, snap.world.y
                        local front       = mob_named("불여우", x - 1, y, 901)
                        local side        = mob_named("불여우", x, y - 1, 902)
                        local round       = mob_named("불여우", x - 1, y - 1, 903)
                        snap.world.mobs   = { front, side, round }
                        snap.buffs        = { "측면공격" }
                        snap.spells       = {}
                        snap.self.hp      = snap.self.max_hp
                        local choice      = battle.choose(snap, bound, front)
                        local hit         = {}
                        for _, splash in ipairs(choice ~= nil and choice.key == "SWING" and choice.extra or {}) do
                            hit[splash.mob.oid] = true
                        end
                        return choice ~= nil and choice.key == "SWING" and hit[902] == true and hit[903] == nil,
                            string.format("key=%s side=%s round=%s", tostring(choice and choice.key), tostring(hit[902]),
                                tostring(hit[903]))
                    end),
                    case("a warrior losing to a crab at full mana swings rather than casting 백열장 that hits no harder", function()
                        local snap, bound = recorded("solo_crab_full_mana")
                        local crab        = nil
                        for _, mob in ipairs(snap.world.mobs) do
                            if mob.oid == 50 then
                                crab = mob
                            end
                        end
                        if crab == nil or snap.self.mp < snap.self.max_mp then
                            return false, "premise: no crab beside a full mana bar"
                        end
                        local choice = battle.choose(snap, bound, crab)
                        return choice ~= nil and choice.key ~= "백열장", tostring(choice and choice.key)
                    end),
                    case("a mob turns on whoever hurt it most, keeps its target on a tie and holds a lure", function()
                        local engine = require("sim.world.engine")
                        local open   = string.rep("\0", 20 * 20)
                        local world  = engine.new({ map = 1, grid = { width = 20, height = 20, blocked = open, collisions = open, spawns = {} }, seed = 1 })
                        local data   = nil
                        for _, mob in pairs(hunting.mobs) do
                            if mob.attack == "AGGRESSIVE" and data == nil then
                                data = mob
                            end
                        end
                        local mob                                         = world:spawn(
                            { spawn = {}, data = data, alive = 0 }, 5, 5)
                        local poet                                        = { oid = 1, x = 5, y = 6, dead = false }
                        local warrior                                     = { oid = 2, x = 5, y = 9, dead = false }
                        world.characters[poet], world.characters[warrior] = true, true
                        mob.locked                                        = world:sighted(mob)
                        local nearest                                     = mob.locked
                        world:remember(mob, warrior, 280)
                        local drawn = mob.locked
                        world:remember(mob, poet, 100)
                        local kept = mob.locked
                        world:remember(mob, poet, 180)
                        local tied = mob.locked
                        world:provoke(mob, poet)
                        world:remember(mob, warrior, 1000)
                        local lured = mob.locked
                        world.now = 6000
                        world:remember(mob, warrior, 1)
                        local after = mob.locked
                        return nearest == poet and drawn == warrior and kept == warrior and tied == warrior and
                            lured == poet and after == warrior,
                            string.format("nearest=%s drawn=%s kept=%s tied=%s lured=%s after=%s",
                                tostring(nearest and nearest.oid),
                                tostring(drawn and drawn.oid), tostring(kept and kept.oid), tostring(tied and tied.oid),
                                tostring(lured and lured.oid), tostring(after and after.oid))
                    end),
                    case("a rescuer draws a mob off its poet with a spell from afar for the share of the lead it closes, not once it leads", function()
                        local function draws(dealt, led)
                            local snap, bound                          = apart_floors("warrior63", "poet65")
                            local poet                                 = state("poet65")
                            snap.world.x, snap.world.y                 = 10, 12
                            poet.world.map, poet.world.x, poet.world.y = snap.world.map, 16, 12
                            local mob                                  = mob_named("칼든해골", 17, 12, 901)
                            poet.world.mobs                            = { mob_named("칼든해골", 17, 12, 901), mob_named(
                                "칼든해골", 16, 13, 902),
                                mob_named("칼든해골", 16, 11, 903) }
                            poet.self.struck                           = 0
                            poet.record.memory.dealt                   = dealt and
                                { ["901"] = { damage = dealt, at = poet.now, map = poet.world.map } } or nil
                            snap.record.memory.dealt                   = led and
                                { ["901"] = { damage = led, at = snap.now, map = snap.world.map } } or nil
                            snap.group                                 = { snap.self.name, poet.self.name }
                            poet.group                                 = snap.group
                            snap.leader                                = snap.self.name
                            snap.party                                 = { party.profile(poet, bound, nil) }
                            snap.world.mobs                            = { mob }
                            local rescue                               = party.rescue(snap, bound)
                            local choice                               = battle.choose(snap, bound, mob, true)
                            return choice and choice.key, rescue ~= nil and rescue.name == poet.self.name
                        end
                        local idle, named = draws(nil)
                        local hard        = draws(1000000)
                        local drawn       = draws(nil, 1000000)
                        local closing     = draws(400)
                        return idle == "백열장" and named and hard ~= "백열장" and drawn ~= "백열장" and closing == "백열장",
                            string.format("idle=%s named=%s hard=%s drawn=%s closing=%s", tostring(idle), tostring(named),
                                tostring(hard), tostring(drawn), tostring(closing))
                    end),
                    case("a bot tallies the damage it dealt each mob for blows that landed until the mob dies or it leaves the floor", function()
                        local blow   = { damage = 300 }
                        local dealt  = battle.tally(nil, 901, blow, "DONE", 1000, 5)
                        dealt        = battle.tally(dealt, 901, blow, "REFUSED", 1500, 5)
                        dealt        = battle.tally(dealt, 901, blow, "DONE", 100000, 5)
                        dealt        = battle.tally(dealt, 902, blow, "DONE", 100000, 5)
                        local kept   = dealt["901"] ~= nil and dealt["901"].damage or 0
                        local killed = battle.tally(dealt, 902, blow, "KILLED", 100500, 5)
                        local moved  = battle.tally(killed, 903, blow, "DONE", 101000, 6)
                        return kept == 600 and killed["902"] == nil and killed["901"] ~= nil and moved["901"] == nil and
                            moved["903"] ~= nil,
                            string.format("kept=%s killed=%s moved=%s/%s", tostring(kept), tostring(killed["902"] ~= nil),
                                tostring(moved["901"] ~= nil), tostring(moved["903"] ~= nil))
                    end),
                    case("a leader keeps its floor while a crewmate there is walled in, not while it can walk", function()
                        local function pick(walled)
                            local first, seen                          = state("rogue50")
                            local team                                 = state("poet65")
                            first.party                                = { party.profile(team, seen, nil) }
                            first.group                                = { team.self.name, first.self.name }
                            first.leader                               = first.self.name
                            local plan                                 = party.plan(first, seen)
                            local snap, bound                          = state("rogue50")
                            local crew                                 = state("poet65")
                            snap.world.map, snap.world.x, snap.world.y = plan.floors[1].map, 10, 10
                            crew.world.map, crew.world.x, crew.world.y = snap.world.map, 12, 10
                            crew.group                                 = { crew.self.name, snap.self.name }
                            local sides                                = walled and
                                { { 13, 10 }, { 11, 10 }, { 12, 9 }, { 12, 11 } } or { { 13, 10 } }
                            local still                                = nil
                            for _, mob in pairs(hunting.mobs) do
                                if mob.attack == "RETALIATE" and (still == nil or mob.name < still.name) then
                                    still = mob
                                end
                            end
                            crew.world.mobs = {}
                            for index, side in ipairs(sides) do
                                table.insert(crew.world.mobs, mob_named(still.name, side[1], side[2], 900 + index))
                            end
                            snap.party                = { party.profile(crew, bound,
                                { task = "FIELD", kind = ACTION.HOLD }) }
                            snap.group                = { crew.self.name, snap.self.name }
                            snap.leader               = snap.self.name
                            snap.record.memory.commit = plan.name
                            snap.world.mobs           = {}
                            snap.buffs                = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "BUFF" then
                                    table.insert(snap.buffs, learned.name)
                                end
                            end
                            data.trace = true
                            decide.choose(snap, bound, nil, nil)
                            local rows = decide.offers[snap.self.name] or {}
                            data.trace = false
                            return rows[1] ~= nil and rows[1].path or "", snap.party[1].walled == true, #plan.floors
                        end
                        local free, open, floors = pick(false)
                        local held, shut         = pick(true)
                        local moved              = string.find(free, "next_floor", 1, true) ~= nil
                        local stayed             = string.find(held, "next_floor", 1, true) == nil
                        return floors > 1 and open == false and shut and moved and stayed,
                            string.format("floors=%d open=%s shut=%s free=%s held=%s", floors, tostring(open),
                                tostring(shut),
                                free, held)
                    end),
                    case("a hitter frees a crewmate walled in by mobs that are no prey, a poet does not spend a heal on it", function()
                        local function pick(me, mate)
                            local first, seen                          = state(me)
                            local team                                 = state(mate)
                            first.party                                = { party.profile(team, seen, nil) }
                            first.group                                = { team.self.name, first.self.name }
                            first.leader                               = first.self.name
                            local plan                                 = party.plan(first, seen)
                            local snap, bound                          = state(me)
                            local crew                                 = state(mate)
                            snap.world.map, snap.world.x, snap.world.y = plan.floors[1].map, 10, 10
                            crew.world.map, crew.world.x, crew.world.y = snap.world.map, 12, 10
                            crew.group                                 = { crew.self.name, snap.self.name }
                            local prey                                 = {}
                            for _, name in ipairs(plan.floors[1].targets or {}) do
                                prey[name] = true
                            end
                            local still = nil
                            for _, mob in pairs(hunting.mobs) do
                                if mob.attack == "RETALIATE" and prey[mob.name] == nil and (still == nil or mob.name < still.name) then
                                    still = mob
                                end
                            end
                            local walls = {}
                            for index, side in ipairs({ { 13, 10 }, { 11, 10 }, { 12, 9 }, { 12, 11 } }) do
                                table.insert(walls, mob_named(still.name, side[1], side[2], 900 + index))
                            end
                            crew.world.mobs           = walls
                            snap.party                = { party.profile(crew, bound,
                                { task = "FIELD", kind = ACTION.HOLD }) }
                            snap.group                = { crew.self.name, snap.self.name }
                            snap.leader               = snap.self.name
                            snap.record.memory.commit = plan.name
                            snap.world.mobs           = walls
                            snap.buffs                = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "BUFF" then
                                    table.insert(snap.buffs, learned.name)
                                end
                            end
                            data.trace = true
                            decide.choose(snap, bound, nil, nil)
                            local rows = decide.offers[snap.self.name] or {}
                            data.trace = false
                            local path = rows[1] ~= nil and rows[1].path or ""
                            return snap.party[1].walled == true and string.find(path, "free_member", 1, true) ~= nil,
                                path
                        end
                        local hitter, struck = pick("rogue50", "poet65")
                        local healer, healed = pick("poet65", "warrior63")
                        return hitter and healer == false,
                            string.format("hitter=%s healer=%s | %s | %s", tostring(hitter), tostring(healer), struck,
                                healed)
                    end),
                    case("a poet walled in beside its 건곤대나이 warrior keeps its mana for heals, alone it casts its way out", function()
                        local defend = require("brain.tasks.defend")
                        local function clear(grouped)
                            local snap, bound = state("poet65")
                            snap.world.mobs   = { mob_named("칼든해골", snap.world.x + 1, snap.world.y, 801) }
                            if grouped then
                                local warrior                                       = state("warrior63")
                                warrior.world.map, warrior.world.x, warrior.world.y = snap.world.map, snap.world.x - 1,
                                    snap.world.y
                                warrior.group                                       = { warrior.self.name, snap.self
                                    .name }
                                snap.group                                          = warrior.group
                                snap.leader                                         = warrior.self.name
                                snap.party                                          = { party.profile(warrior, bound, nil) }
                            end
                            return defend.clear_path(snap, bound)
                        end
                        local alone  = clear(false)
                        local crewed = clear(true)
                        return alone ~= nil and alone.key ~= "SWING" and (crewed == nil or crewed.key == "SWING"),
                            string.format("alone=%s crewed=%s", tostring(alone and alone.key),
                                tostring(crewed and crewed.key))
                    end),
                    case("a leader keeps its floor while a crewmate there cannot move, whatever the crewmate does", function()
                        local function pick(pinned)
                            local first, seen                          = state("rogue50")
                            local team                                 = state("poet65")
                            first.party                                = { party.profile(team, seen, nil) }
                            first.group                                = { team.self.name, first.self.name }
                            first.leader                               = first.self.name
                            local plan                                 = party.plan(first, seen)
                            local snap, bound                          = state("rogue50")
                            local crew                                 = state("poet65")
                            snap.world.map, snap.world.x, snap.world.y = plan.floors[1].map, 10, 10
                            crew.world.map, crew.world.x, crew.world.y = snap.world.map, 11, 10
                            crew.world.mobs                            = {}
                            crew.group                                 = { crew.self.name, snap.self.name }
                            snap.party                                 = { party.profile(crew, bound,
                                { task = "FIELD", kind = ACTION.HOLD }) }
                            snap.party[1].pinned                       = pinned or nil
                            snap.group                                 = { crew.self.name, snap.self.name }
                            snap.leader                                = snap.self.name
                            snap.record.memory.commit                  = plan.name
                            snap.world.mobs                            = {}
                            snap.buffs                                 = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "BUFF" then
                                    table.insert(snap.buffs, learned.name)
                                end
                            end
                            data.trace = true
                            decide.choose(snap, bound, nil, nil)
                            local rows = decide.offers[snap.self.name] or {}
                            data.trace = false
                            return rows[1] ~= nil and rows[1].path or "", #plan.floors
                        end
                        local free, floors = pick(false)
                        local held         = pick(true)
                        return floors > 1 and string.find(free, "next_floor", 1, true) ~= nil and
                            string.find(held, "next_floor", 1, true) == nil,
                            string.format("floors=%d free=%s held=%s", floors, free, held)
                    end),
                    case("a leader at the entrance waits for a crewmate that cannot move instead of fetching it", function()
                        local snap, bound                          = state("warrior63")
                        local poet, seen                           = state("poet65")
                        snap.world.map, snap.world.x, snap.world.y = 22, 147, 17
                        poet.world.map, poet.world.x, poet.world.y = 22, 60, 150
                        snap.group                                 = { snap.self.name, poet.self.name }
                        snap.leader                                = snap.self.name
                        snap.record.memory.commit                  = "국내해골굴"
                        snap.party                                 = { party.profile(poet, seen, nil) }
                        snap.party[1].task                         = "FIELD"
                        snap.buffs                                 = {}
                        for _, learned in ipairs(snap.spells) do
                            local spell = spells.spells[learned.name]
                            if spell ~= nil and spell.role == "BUFF" then
                                table.insert(snap.buffs, learned.name)
                            end
                        end
                        local function pick(pinned)
                            snap.party[1].pinned = pinned or nil
                            data.trace           = true
                            local chosen         = decide.choose(snap, bound, nil, nil)
                            local rows           = decide.offers[snap.self.name] or {}
                            data.trace           = false
                            return chosen, rows[1] ~= nil and rows[1].path or ""
                        end
                        local held, path = pick(true)
                        local dest       = held ~= nil and held.dest or {}
                        return held ~= nil and string.find(path, "lead_gate", 1, true) ~= nil and
                            (held.kind == ACTION.HOLD or (dest.x ~= nil and math.abs(dest.x - 147) + math.abs(dest.y - 17) <= data.adjacent)),
                            string.format("%s/%s dest=%s(%s,%s) path=%s", tostring(held and held.task),
                                tostring(held and held.kind), tostring(dest.map), tostring(dest.x), tostring(dest.y),
                                path)
                    end),
                    case("a member that cannot move walks its blocked walk again instead of waiting for its crew", function()
                        local function pick(pinned)
                            local first, seen                          = state("rogue50")
                            local team                                 = state("poet65")
                            first.party                                = { party.profile(team, seen, nil) }
                            first.group                                = { team.self.name, first.self.name }
                            first.leader                               = first.self.name
                            local plan                                 = party.plan(first, seen)
                            local lead                                 = state("rogue50")
                            local snap, bound                          = state("poet65")
                            lead.world.map, lead.world.x, lead.world.y = plan.floors[1].map, 10, 10
                            snap.world.map, snap.world.x, snap.world.y = lead.world.map, 11, 10
                            lead.world.mobs, snap.world.mobs           = {}, {}
                            lead.group                                 = { lead.self.name, snap.self.name }
                            lead.leader                                = lead.self.name
                            lead.party                                 = first.party
                            lead.record.memory.commit                  = plan.name
                            local buffs                                = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "BUFF" then
                                    table.insert(buffs, learned.name)
                                end
                            end
                            lead.buffs, snap.buffs    = buffs, buffs
                            snap.party                = { party.profile(lead, seen,
                                { task = "FIELD", kind = ACTION.HOLD }) }
                            snap.group                = lead.group
                            snap.leader               = lead.self.name
                            snap.record.memory.commit = plan.name
                            if pinned then
                                snap.record.memory.spot    = { map = snap.world.map, x = 11, y = 10, at = snap.now - 5000 }
                                snap.record.memory.blocked = {
                                    map = snap.world.map,
                                    at = snap.now - 1000,
                                    since = snap.now - 5000,
                                    dest = { map = snap.world.map, x = 11, y = 12, range = 0 }
                                }
                            end
                            data.trace   = true
                            local chosen = decide.choose(snap, bound, nil, nil)
                            local rows   = decide.offers[snap.self.name] or {}
                            data.trace   = false
                            return chosen, rows[1] ~= nil and rows[1].path or ""
                        end
                        local free, waited  = pick(false)
                        local held, retried = pick(true)
                        local dest          = held ~= nil and held.dest or {}
                        return string.find(waited, "retry_walk", 1, true) == nil and
                            string.find(retried, "retry_walk", 1, true) ~= nil and dest.x == 11 and dest.y == 12,
                            string.format("free=%s/%s held=%s/%s", tostring(free and free.kind), waited,
                                tostring(held and held.kind), retried)
                    end),
                    case("a poet sent to save a crewmate beside its 건곤대나이 warrior keeps its mana, away from it it strikes", function()
                        local function pick(near)
                            local first, seen                                 = state("warrior63")
                            local team                                        = state("poet65")
                            first.party                                       = { party.profile(team, seen, nil) }
                            first.group                                       = { team.self.name, first.self.name }
                            first.leader                                      = first.self.name
                            local plan                                        = party.plan(first, seen)
                            local snap, bound                                 = state("poet65")
                            local lead                                        = state("warrior63")
                            local mage                                        = state("mage60")
                            snap.world.map, snap.world.x, snap.world.y        = plan.floors[1].map, 10, 2
                            lead.world.map, lead.world.x, lead.world.y        = snap.world.map, near and 9 or 26, 2
                            mage.world.map, mage.world.x, mage.world.y        = snap.world.map, 12, 2
                            local mob                                         = mob_named("칼든해골", 13, 2, 801)
                            snap.world.mobs, lead.world.mobs, mage.world.mobs = { mob }, {}, {}
                            local group                                       = { lead.self.name, snap.self.name, mage
                                .self.name }
                            lead.group, lead.leader                           = group, lead.self.name
                            lead.party                                        = first.party
                            lead.record.memory.commit                         = plan.name
                            local buffs                                       = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "BUFF" then
                                    table.insert(buffs, learned.name)
                                end
                            end
                            lead.buffs, mage.buffs, snap.buffs = buffs, buffs, buffs
                            snap.party                         = {
                                party.profile(lead, seen, { task = "FIELD", kind = ACTION.HOLD }),
                                party.profile(mage, seen, { task = "DEFEND", kind = ACTION.ATTACK }),
                            }
                            snap.party[1].rescue               = {
                                name = mage.self.name, by = snap.self.name, value = 1e6, shares = { ["801"] = 1 }
                            }
                            local applied                      = {}
                            for _, learned in ipairs(snap.spells) do
                                local spell = spells.spells[learned.name]
                                if spell ~= nil and spell.role == "DEBUFF" then
                                    applied[learned.name] = snap.now
                                end
                            end
                            snap.record.memory.battle = { oid = 801, map = snap.world.map, applied = applied }
                            snap.group, snap.leader   = group, lead.self.name
                            snap.record.memory.commit = plan.name
                            local found, path         = require("brain.tree").run(require("brain.trees.hunt"), snap,
                                bound)
                            return found ~= nil and found.action or nil, table.concat(path or {}, ">")
                        end
                        local kept, beside = pick(true)
                        local cast, apart  = pick(false)
                        return string.find(apart, "rescue", 1, true) ~= nil and cast ~= nil and cast.key ~= nil and
                            cast.key ~= "SWING" and string.find(beside, "rescue", 1, true) == nil and
                            (kept == nil or kept.key ~= cast.key),
                            string.format("beside=%s/%s %s | apart=%s/%s %s", tostring(kept and kept.kind),
                                tostring(kept and kept.key), beside, tostring(cast and cast.kind),
                                tostring(cast and cast.key),
                                apart)
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
                    case("a member beside the vanguard waits for it to cross and follows once it is over", function()
                        local probe, probed = apart_floors("poet65", "rogue50")
                        local plan = party.plan(probe, probed)
                        local first, second = plan.floors[1].map, plan.floors[2] and plan.floors[2].map
                        local snap, bound = apart_floors("poet65", "rogue50")
                        local lead = snap.party[1]
                        snap.world.map, snap.world.x, snap.world.y, snap.world.mobs = first, 10, 10, {}
                        lead.world.map, lead.world.x, lead.world.y = first, 11, 10
                        lead.heading, lead.task = { second }, "FIELD"
                        local waiting = decide.choose(snap, bound, nil, nil)
                        local after, followed = apart_floors("poet65", "rogue50")
                        local over = after.party[1]
                        after.world.map, after.world.x, after.world.y, after.world.mobs = first, 10, 10, {}
                        over.world.map, over.world.x, over.world.y = second, 5, 5
                        over.heading, over.task = nil, "FIELD"
                        local pick = decide.choose(after, followed, nil, nil)
                        local maps = pick ~= nil and pick.dest ~= nil and (pick.dest.maps or { pick.dest.map }) or {}
                        return second ~= nil and waiting ~= nil and waiting.kind == ACTION.HOLD and pick ~= nil and
                            pick.kind == ACTION.MOVE and maps[1] == second,
                            string.format("waiting=%s task=%s kind=%s dest=%s next=%s",
                                tostring(waiting and waiting.kind),
                                tostring(pick and pick.task), tostring(pick and pick.kind), tostring(maps[1]),
                                tostring(second))
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
                    case("after a flight the vanguard at the gate leads its crew in", function()
                        local snap, bound, lead, plan = after_flight("rogue50", "poet65")
                        local gate = party.entry_gate(snap, bound, plan)
                        snap.world.map = gate
                        lead.world.map, lead.world.x, lead.world.y = gate, snap.world.x + 1, snap.world.y
                        lead.heading, lead.task = nil, "FIELD"
                        data.trace = true
                        local pick = decide.choose(snap, bound, nil, nil)
                        data.trace = false
                        local rows = decide.offers[snap.self.name] or {}
                        local path = rows[1] ~= nil and rows[1].path or ""
                        return pick ~= nil and pick.kind == ACTION.MOVE and
                            string.find(path, "lead_gate", 1, true) ~= nil,
                            string.format("task=%s kind=%s path=%s", tostring(pick and pick.task),
                                tostring(pick and pick.kind), path)
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
                    case("watch: a member walking to its crewmate's floor is not split", function()
                        local snap, bound = apart_floors("rogue50", "poet65")
                        local there       = snap.party[1].world.map
                        return dogged(snap, bound,
                            repeated(6, 10000, { task = "FIELD", kind = ACTION.MOVE, dest = { maps = { there } } },
                                "x>rally", function(s, actor)
                                    s.self.mp = actor.clock % 100
                                end)).split == nil, "split"
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
                    case("a vanguard stays with a crewmate shooting a mob across a wall, not walking round nor moving on", function()
                        local function pick(shooting)
                            local snap, bound = recorded("healer_waits_beside_leader")
                            snap.bag          = driver.stocked(snap.bag.capacity)
                            local mate        = snap.party[1]
                            mate.dealt        = shooting and
                                { [tostring(mate.battle.oid)] = { damage = 1000, at = snap.now } } or {}
                            data.trace        = true
                            local chosen      = decide.choose(snap, bound, nil, nil)
                            local rows        = decide.offers[snap.self.name] or {}
                            data.trace        = false
                            local dest        = chosen ~= nil and chosen.dest or {}
                            local far         = dest.x ~= nil and
                                math.abs(dest.x - mate.world.x) + math.abs(dest.y - mate.world.y) > data.adjacent
                            return rows[1] ~= nil and rows[1].path or "", dest.maps == nil and far == false
                        end
                        local stayed, kept = pick(true)
                        local cursed       = pick(false)
                        return stayed ~= "" and string.find(stayed, "next_floor", 1, true) == nil and
                            string.find(stayed, "strike", 1, true) == nil and kept and
                            string.find(cursed, "wait_leader", 1, true) == nil,
                            string.format("shooting=%s cursed=%s", stayed, cursed)
                    end),
                    case("a leader takes the rally to a crewmate walled in four tiles off and walks to it", function()
                        local snap, bound = recorded("rogue_waits_walled_poet")
                        local poet        = party.members(snap)[1]
                        if party.held(poet) == false then
                            return false, "premise: the poet is not held"
                        end
                        local rally  = party.rally(snap, bound)
                        local chosen = decide.choose(snap, bound, nil, nil)
                        local dest   = chosen ~= nil and chosen.dest or {}
                        return rally ~= nil and rally.name == poet.self.name and dest.x == poet.world.x and
                            dest.y == poet.world.y,
                            string.format("rally=%s dest=%s,%s", tostring(rally and rally.name), tostring(dest.x),
                                tostring(dest.y))
                    end),
                    case("a member the leader sent a crewmate to save fights where it stands instead of walking to the rally", function()
                        local function pick(saved)
                            local snap, bound = recorded("poet_walled_rally")
                            local lead        = snap.party[1]
                            lead.rescue       = saved and { name = snap.self.name, by = lead.self.name, value = 1 } or
                                nil
                            local chosen      = decide.choose(snap, bound, nil, nil)
                            local dest        = chosen ~= nil and chosen.dest or {}
                            return dest.x == lead.rally.x and dest.y == lead.rally.y, chosen and chosen.task
                        end
                        local walked, before = pick(false)
                        local stayed, task   = pick(true)
                        return walked and stayed == false,
                            string.format("unsaved=%s/%s saved=%s/%s", tostring(walked), tostring(before),
                                tostring(stayed), tostring(task))
                    end),
                    case("a member a floor ahead on its vanguard's way waits there, and walks back only to one that cannot move", function()
                        local function pick(pinned)
                            local snap, bound    = recorded("swap_pass_through")
                            snap.world.map       = 3002
                            snap.world.x         = 2
                            snap.world.y         = 17
                            snap.world.mobs      = {}
                            snap.party[1].pinned = pinned or nil
                            data.trace           = true
                            local chosen         = decide.choose(snap, bound, nil, nil)
                            local rows           = decide.offers[snap.self.name] or {}
                            data.trace           = false
                            local dest           = chosen ~= nil and chosen.dest or {}
                            return rows[1] ~= nil and rows[1].path or "", dest.maps ~= nil and dest.maps[1] == 3001
                        end
                        local waited, left  = pick(false)
                        local fetched, went = pick(true)
                        return left == false and went, string.format("waited=%s fetched=%s", waited, fetched)
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
                    case("a solo fighter whose own heal falls short of its ground still plans hp supplies", function()
                        local snap, bound = recorded("rogue_no_food")
                        local need = require("brain.tasks.supply").need_of(snap, bound, "hp")
                        return need ~= nil and need.gain > 0 and need.usage > 0,
                            string.format("gain=%s usage=%s", tostring(need and need.gain), tostring(need and need.usage))
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
                    case("crew escape leaves the ground like a doomed flight", function()
                        local snap, bound = recorded("rogue_flees_on_foot")
                        snap.self.hp = 858
                        local escape = require("brain.tasks.escape")
                        local mob = escape.adjacent_hitter(snap)
                        local crew = escape.flee(snap, bound, mob, "CREW").dest
                        local doomed = escape.flee(snap, bound, mob, "DOOMED").dest
                        return crew.urgent == true and doomed.urgent == true and crew.x == nil and
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
                    case("mana that adds nothing to its hunt gives a rogue no reason to drink", function()
                        local snap, bound = recorded("rogue_dry_with_doses")
                        local hits        = swings_watched_dry(snap, bound)
                        local worth       = require("brain.policies.value").mana(snap, bound)
                        return worth == 0 and hits.dry_with_doses == nil,
                            string.format("mana=%.4f flagged=%s", worth, tostring(hits.dry_with_doses))
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
                        without(warrior, "측면공격")
                        without(warrior, "후면공격")
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
                    case("a pile behind a wall is weighed by the walk through the door, here and two steps on", function()
                        local snap, bound = recorded("pile_behind_wall")
                        local loot        = require("brain.tasks.loot")
                        local now         = snap.now
                        snap.now          = now - 60000
                        loot.pile(snap, bound)
                        snap.now   = now
                        local pile = loot.pile(snap, bound)
                        if pile == nil then
                            return false, "premise: no pile"
                        end
                        local direct = math.abs(pile.x - snap.world.x) + math.abs(pile.y - snap.world.y)
                        local walked = bound.steps(snap.world.map, snap.world.x, snap.world.y,
                            { { x = pile.x, y = pile.y, range = 0 } })[1]
                        local here   = loot.value(snap, bound)
                        local pick   = decide.choose(snap, bound, nil, nil)
                        return type(walked) == "number" and walked > direct and here < 0 and pick ~= nil and
                            pick.task ~= "LOOT",
                            string.format("direct=%s walked=%s value=%.0f task=%s", tostring(direct), tostring(walked),
                                here,
                                tostring(pick and pick.task))
                    end),
                    case("a bot whose hp two volleys of the mobs on it would end eats before it flees", function()
                        local snap, bound = recorded("flees_before_eating")
                        local burst       = battle.burst(snap)
                        local plan        = party.plan(snap, bound)
                        if plan == nil or 2 * burst <= 2 * plan.peak or snap.self.hp > 2 * burst then
                            return false, "premise: the mobs on it strike no harder than the plan foresaw"
                        end
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "REST" and pick.kind == ACTION.USE,
                            string.format("task=%s kind=%s item=%s hp=%s burst=%.0f peak=%.0f",
                                tostring(pick and pick.task),
                                tostring(pick and pick.kind), tostring(pick and pick.item), tostring(snap.self.hp), burst,
                                plan.peak)
                    end),
                    case("a hurt fighter whose strike is not worth its fight still strikes beside its healer", function()
                        local snap, bound = recorded("fighter_stands_awaiting")
                        local defend      = require("brain.tasks.defend")
                        local restore     = require("brain.trees.restore")
                        if defend.value(snap, bound) >= 0 or restore.awaiting(snap, bound) == false then
                            return false, "premise: the strike is worth it or no heal is awaited"
                        end
                        local pick = decide.choose(snap, bound, nil, nil)
                        return pick ~= nil and pick.task == "DEFEND",
                            string.format("kind=%s task=%s key=%s", tostring(pick and pick.kind),
                                tostring(pick and pick.task),
                                tostring(pick and pick.key))
                    end),
                    case("a fighter under blows does not wait for a heal its poet's mana cannot cover", function()
                        local snap, bound = recorded("fighter_awaits_dry_healer")
                        local crew        = require("brain.policies.escort").present(snap, bound)
                        local pick        = decide.choose(snap, bound, nil, nil)
                        return crew ~= nil and crew.stock < snap.self.max_hp - snap.self.hp and pick ~= nil and
                            (pick.kind ~= ACTION.HOLD or (pick.pause or 0) > 0),
                            string.format("kind=%s task=%s item=%s pause=%s hp=%s stock=%s", tostring(pick and pick.kind),
                                tostring(pick and pick.task), tostring(pick and pick.item), tostring(pick and pick.pause),
                                tostring(snap.self.hp), tostring(crew and crew.stock))
                    end),
                    case("a poet short of mana charges among skeletons even when the charge leaves less than the next volley", function()
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
                        return string.find(path, "drink", 1, true) == nil,
                            path .. " charged=" .. recovery.charged(snap) ..
                            " burst=" .. battle.burst(snap)
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
                        snap.buffs = buffed(snap)
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
                    case("a fight plan takes no less time than the casts it makes, healed or not", function()
                        local snap, bound = state("warrior63")
                        local plan        = ground.choose(snap, bound)
                        local mob         = mob_named(plan.floors[1].targets[1], snap.world.x + 1, snap.world.y, 204)
                        local cast        = combat.cast(snap)
                        local worst       = nil
                        local struck      = 0
                        for _, aid in ipairs({ 0, 0.5, 1, 2, 4 }) do
                            for step = 1, 40 do
                                local prey = {}
                                for key, entry in pairs(mob) do
                                    prey[key] = entry
                                end
                                prey.ratio   = step / 40
                                local result = combat.outcome(snap, bound, prey, nil, aid)
                                struck       = struck + ((result.spent or 0) > 0 and 1 or 0)
                                if result.time + 1e-6 < result.casts * cast and worst == nil then
                                    worst = string.format("aid=%s ratio=%s time=%s casts=%s", aid, prey.ratio,
                                        result.time,
                                        result.casts)
                                end
                            end
                        end
                        return worst == nil and struck > 0, tostring(worst) .. " struck=" .. struck
                    end),
                    case("a strike paid with hp is spent on a mob only for the time it saves, and its hp counted once", function()
                        local snap, bound = state("warrior63")
                        local plan        = ground.choose(snap, bound)
                        local function fought(ratio, aid)
                            local prey = mob_named(plan.floors[1].targets[1], snap.world.x + 1, snap.world.y, 205)
                            prey.ratio = ratio
                            return combat.outcome(snap, bound, prey, nil, aid)
                        end
                        local ended = fought(1 / 40, 0.5)
                        local paid  = nil
                        for _, aid in ipairs({ 1, 2, 4, 8 }) do
                            local result = fought(1, aid)
                            paid         = paid or ((result.spent or 0) > 0 and result or nil)
                        end
                        local margin = paid ~= nil and (snap.self.hp + paid.healed - paid.taken) / snap.self.max_hp or
                            nil
                        return (ended.spent or 0) == 0 and paid ~= nil and math.abs(paid.margin - margin) < 1e-9,
                            string.format("ended=%s paid=%s margin=%s/%s healed=%s", tostring(ended.spent),
                                tostring(paid and paid.spent), tostring(paid and paid.margin), tostring(margin),
                                tostring(paid and paid.healed))
                    end),
                    case("a rogue among foxes beside its crew values 투명 no lower than a swing still on its cooldown", function()
                        local snap, bound = recorded("rogue_cloak_foxes")
                        data.trace        = true
                        local mob         = nil
                        for _, seen in ipairs(snap.world.mobs) do
                            mob = mob or (seen.oid == 3 and seen or nil)
                        end
                        local found  = battle.choose(snap, bound, mob)
                        data.trace   = false
                        local values = {}
                        for _, entry in ipairs(found and found.rivals or {}) do
                            values[entry.key] = entry.value
                        end
                        return values["투명"] ~= nil and values.SWING ~= nil and values["투명"] >= values.SWING,
                            string.format("cloak=%s swing=%s pick=%s", tostring(values["투명"]), tostring(values.SWING),
                                tostring(found and found.key))
                    end),
                    case("a mob my walk could not reach is struck from where I stand, one my spell was refused at is not", function()
                        local function aimed(cast)
                            local snap, bound              = state("mage60")
                            local plan                     = ground.choose(snap, bound)
                            local mob                      = mob_named(plan.floors[1].targets[1], snap.world.x + 3,
                                snap.world.y, 203)
                            snap.world.mobs                = { mob }
                            snap.record.memory.unreachable = {
                                [tostring(mob.oid)] = { map = snap.world.map, x = mob.x, y = mob.y, at = snap.now, cast = cast }
                            }
                            return battle.target(snap, bound, any)
                        end
                        local walked                   = aimed(false)
                        local refused                  = aimed(true)
                        local snap, bound              = state("warrior63")
                        local plan                     = ground.choose(snap, bound)
                        local mob                      = mob_named(plan.floors[1].targets[1], snap.world.x + 3,
                            snap.world.y, 207)
                        snap.world.mobs                = { mob }
                        snap.self.mp                   = snap.self.max_mp - 30
                        snap.record.memory.unreachable = {
                            [tostring(mob.oid)] = { map = snap.world.map, x = mob.x, y = mob.y, at = snap.now }
                        }
                        local hitter                   = battle.target(snap, bound, any)
                        local maneuver                 = hitter ~= nil and hitter.choice.maneuver or nil
                        local shot                     = maneuver == nil or
                            (maneuver.kind == MANEUVER.CAST and maneuver.self ~= true)
                        return walked ~= nil and walked.choice.approach ~= true and refused == nil and shot,
                            string.format("walked=%s refused=%s hitter=%s", tostring(walked and walked.choice.key),
                                tostring(refused and refused.choice.key), tostring(hitter and hitter.choice.key))
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
                    case("a walk past a slower mob that kills in one blow is not free three tiles off", function()
                        local danger      = require("brain.policies.danger")
                        local snap, bound = state("rogue92")
                        local mob         = mob_named("황담비", snap.world.x + 3, snap.world.y, 101)
                        local near        = danger.passing(snap, bound, mob, 3, 4000)
                        local far         = danger.passing(snap, bound, mob, 9, 4000)
                        return near > 0.5 and far == 0, string.format("near=%.3f far=%.3f", near, far)
                    end),
                    case("a mob known to bear nothing is not fought as one whose debuffs nobody knows", function()
                        local snap, bound = state("poet65")
                        local function doll(changes)
                            local mob = mob_named("양철인형", snap.world.x + 1, snap.world.y, 101)
                            for key, value in pairs(changes) do
                                mob[key] = value
                            end
                            return mob
                        end
                        local bare     = combat.outcome(snap, bound, doll({ applied = {} }))
                        local unknown  = combat.outcome(snap, bound, doll({}))
                        local assisted = combat.outcome(snap, bound, doll({ assist = { ["혼마술"] = true } }))
                        local again    = combat.outcome(snap, bound, doll({ assist = { ["혼마술"] = true } }))
                        return unknown.casts > bare.casts and again == assisted,
                            string.format("casts=%d/%d shared=%s", bare.casts, unknown.casts,
                                tostring(again == assisted))
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
