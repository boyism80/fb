local data     = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local score    = require("brain.policies.score")
local value    = require("brain.policies.value")

local M = {}

local journaled = {}

function M.castable(snap, query, mob)
    local least = recovery.least(snap)
    if least == nil or (snap.self.mp or 0) < least then
        return true
    end
    local armed = false
    for _, choice in ipairs(combat.options(snap, mob).offense) do
        armed = true
        if choice.damage > 0 then
            return true
        end
    end
    return armed == false
end

local function approachable(snap, mob)
    local distance = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y)
    return distance <= 1 or combat.hits(mob) == false or
        snap.self.hp > combat.spike(snap, { { mob = mob, count = 1 } })
end

-- A mob I would walk to strike is in a crewmate's reach when the crewmate can walk to the fight (escort.tethered),
-- not by a straight line across walls; one I strike from where I stand is in reach by the straight line, and across
-- a wall it cannot strike back (user, 11-11). Only a spell aimed at it from here strikes from where I stand: a fight
-- that opens with a drink or a buff goes on with my blows, and taken for one from here, a vanguard took a mob 65
-- steps round a wall from its poet for its 웅담 (brain unit test, vanguard_wall_prey).
local function walking(snap, query, mob)
    local choice = battle.choose(snap, query, mob)
    if choice == nil or choice.approach == true then
        return true
    end
    local aimed = choice.aimed == true or (choice.maneuver.kind == MANEUVER.CAST and choice.maneuver.self ~= true)
    return aimed == false or choice.moves == true
end

M.walking = walking

-- A fight I walk to stays within a straight sight's reach of every crewmate, and whether the crewmate can walk there is
-- tethered's to judge. Counting that walk here too, a warrior took six skeletons 8-16 steps from its poet for out of
-- reach, left for the next floor, and they caught the poet on the way (offline profile pair rooted striker seed 0,
-- 579-587s, run 08:53; user 11-11). A strike cast from where I stand needs only every crewmate's sight round the mob
-- (escort.tethered, user 11-11 and 11-15): held to this tighter reach as well, a mage never shot a skeleton at 1% six
-- tiles off across a wall because its warrior stood eight tiles from it, poisoned it again and again instead, and its
-- crew waited on it for five minutes (online 004028, 00:43:36-00:48:30). A mob beside me is no strike from afar and
-- keeps the tighter reach: let through as one, a rogue followed a skeleton to nine tiles from its poet, fought it there,
-- was drawn on by the next ones and stood 57s out of its poet's sight (offline watch trio seed 4, 274-348s, run
-- 031445).
local function near_crew(snap, mob)
    local range = math.min(data.sight.x, data.sight.y)
    for _, member in ipairs(party.members(snap)) do
        local world = member.world
        if member.self.ghost == false and world.map == snap.world.map and world.x ~= nil and
            math.abs(mob.x - world.x) + math.abs(mob.y - world.y) > range then
            return false
        end
    end
    return true
end

local function sought(snap, query, names)
    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end
    -- The vanguard may walk to a crewmate's fight wherever it is: with only its own prey in reach, a warrior in front
    -- walked on to the next floor while the caster beside it shot a skeleton from afar (healer_waits_beside_leader).
    -- That fight is weighed beside its own prey, by the kill, the share of saving the crewmate and the draw off it
    -- (battle.target), not taken first: taken first, a warrior left 88 of 161 fights for the mob its mage had just
    -- poisoned, which no blow of its own could draw off the mage, and walked between them while skeletons at 1% piled
    -- up (online 20261115-181437, 흑해골굴; user 11-16: guarding is worth its draw, never first). Every other hitter takes
    -- its own prey within the crew's reach: with three or more, all striking the one mob the leader fights wastes the
    -- other hitters (crew rule, 11-10).
    local shared = {}
    if party.vanguard_name(snap, query) == snap.self.name then
        for _, member in ipairs(party.members(snap)) do
            local fight = member.self.ghost == false and member.battle or nil
            if type(fight) == "table" and fight.map == snap.world.map then
                shared[fight.oid] = true
            end
        end
    end
    -- A crewmate that heals the others does not lead with attacks (user, 11-06): it strikes only what it reaches from
    -- where it stands. Walking to its own prey, a poet went nine steps for a skeleton beside its warrior's passing
    -- spot while the warrior walked on to the next floor, and was left alone among skeletons (offline profile pair
    -- rooted healer seed 1, 687-697s, run 08:08).
    local healer = #party.members(snap) > 0 and party.vanguard_name(snap, query) ~= snap.self.name and
        score.healer(snap, query)
    -- Whether I would walk to the mob is my best move at it, so it is asked last and only where a crewmate's reach
    -- hangs on it (escort.tethered). Asked first of every mob in sight, it was weighed for bots with no crew at all:
    -- 107,047 asks, a fifth of the time spent deciding (offline 8 teams, run 19:36).
    return battle.target(snap, query, function(mob)
        if wanted[mob.name] == nil or M.castable(snap, query, mob) == false or approachable(snap, mob) == false then
            return false
        end
        local walk = nil
        local function walks()
            if walk == nil then
                walk = walking(snap, query, mob)
            end
            return walk
        end
        if shared[mob.oid] then
            return escort.tethered(snap, query, mob.x, mob.y, walks)
        end
        local beside = math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) <= 1
        if near_crew(snap, mob) == false then
            return beside == false and M.fightable(snap, query, mob) and walks() == false
        end
        return (healer == false or beside or walks() == false) and escort.tethered(snap, query, mob.x, mob.y, walks)
    end)
end

function M.fightable(snap, query, mob)
    return escort.tethered(snap, query, mob.x, mob.y, function()
        return false
    end)
end

-- One hunt target a snapshot: the heal weighed out of a fight asks for it as the hunt does (aid.forgone), and asked
-- twice the first ask also took the forecast sample the hunt's strike was due (battle.target): 5,217 of 7,829
-- picks were the heal's (offline 8 teams, run 19:36).
local targets = setmetatable({}, { __mode = "k" })

function M.target(snap, query, names)
    local known = targets[snap]
    if known == nil then
        known         = {}
        targets[snap] = known
    end
    if known[names] == nil then
        known[names] = sought(snap, query, names) or false
    end
    return known[names] or nil
end

function M.revisit(snap)
    local fleeing = snap.record.memory.fleeing
    return fleeing ~= nil and fleeing.ends > snap.now and fleeing.ends - snap.now or nil
end

function M.wanted(snap, query)
    if snap.self.ghost then
        return false
    end
    local objective = value.objective(snap, query)
    if objective.task ~= "FIELD" or objective.dest == nil or value.recoiling(snap, query, objective.dest) then
        return false
    end
    local plan = party.plan(snap, query)
    local crew = escort.present(snap, query)
    if data.journal and plan ~= nil and journaled[snap.self.name or ""] ~= plan.name then
        journaled[snap.self.name or ""] = plan.name
        local others = {}
        for _, member in ipairs(party.members(snap)) do
            table.insert(others, string.format("%s:%s/%s:%s:%s", member.self.level, member.self.hp, member.self.mp,
                tostring(member.world.map), table.concat(member.buffs or {}, "+")))
        end
        log("debug",
            string.format("[brain:plan] bot=%s lv=%s map=%s plan=%s rate=%.0f hp=%s mp=%s commit=%s buffs=%s others=%s",
                snap.self.name or "", tostring(snap.self.level), tostring(snap.world.map), plan.name, plan.rate,
                tostring(snap.self.hp), tostring(snap.self.mp), tostring(snap.record.memory.commit),
                table.concat(snap.buffs or {}, "+"), table.concat(others, ";")))
        local worn = {}
        for _, slot in ipairs(snap.gear or {}) do
            table.insert(worn, slot.names[1])
        end
        local held = {}
        for _, item in ipairs(snap.bag.items or {}) do
            table.insert(held, item.name .. "x" .. item.count)
        end
        local learned = 0
        for _ in ipairs(snap.spells or {}) do
            learned = learned + 1
        end
        log("debug",
            string.format(
                "[brain:plan] detail max=%s/%s str=%s dex=%s int=%s hit=%s dam=%s def=%s money=%s spells=%d step=%s swing=%s demand=%s gear=%s bag=%s",
                tostring(snap.self.max_hp), tostring(snap.self.max_mp), tostring(snap.self.str), tostring(snap.self.dex),
                tostring(snap.self.int),
                tostring(snap.self.hit), tostring(snap.self.dam), tostring(snap.self.phydef), tostring(snap.self.money),
                learned,
                tostring(snap.pace.step), tostring(snap.pace.swing), tostring(query.demand(plan.floors[1].map)),
                table.concat(worn, ","), table.concat(held, ",")))
    end
    -- Short hp holds the walk to the ground back only while some way to recover pays for itself. Waiting for
    -- the full bar made a 52 hp mage, who lost 1 hp to every 동동주, walk to the shrine and back for good
    -- once natural regeneration no longer refilled it (state supplier, online).
    local hp = snap.self.hp + (crew ~= nil and crew.heal or 0) * combat.cast(snap) / 1000
    return snap.self.hp >= snap.self.max_hp or hp > (plan ~= nil and plan.peak or 0) or
        require("brain.trees.restore").mendable(snap, query) == false
end

return M
