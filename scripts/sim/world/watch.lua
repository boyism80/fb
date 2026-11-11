local data     = require("brain.data.combat")
local battle   = require("brain.policies.battle")
local combat   = require("brain.policies.combat")
local escort   = require("brain.policies.escort")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local value    = require("brain.policies.value")
local rest     = require("brain.tasks.rest")

local M = {}

M.IDLE         = 60000
M.SPLIT        = 45000
M.WAIT_FROM    = 300000
M.WAIT_SHARE   = 0.4
M.KILLS_FROM   = 300000
M.FRAIL_FROM   = 120000
M.FRAIL_RATIO  = 0.25
M.FRAIL_SHARE  = 0.5
M.DRY          = 30000
M.EARLY        = 60000
M.FLIP_WINDOW  = 300000
M.FLIPS        = 3
M.REPEAT       = 3
M.STUCK_ESCAPE = 5
M.SWINGS       = 5

local WORKING = { care = true, weigh = true, rescue = true, strike = true, cure = true, cure_first = true }
local TRIPS   = { OUTFIT = true, LEARN = true, SELL = true }

local function counted(dog, name, active)
    dog[name] = active and (dog[name] or 0) + 1 or 0
    return dog[name]
end

local function last_node(path)
    return (tostring(path or ""):match("([^>]+)$")) or ""
end

local function outswings(snap, bound)
    local neutral = {}
    local spell   = 0
    for _, choice in ipairs(combat.options(snap, neutral).offense) do
        spell = math.max(spell, choice.damage)
    end
    return combat.strike(snap, bound, neutral) >= spell
end

function M.check(actor, snap, bound, decision, path)
    local dog = actor.dog
    if dog == nil then
        dog = { held = {}, flips = {} }
        actor.dog = dog
    end
    local now  = actor.clock
    local dt   = now - (dog.at or now)
    dog.at     = now
    local me   = snap.self
    local task = decision ~= nil and decision.task or nil
    local kind = decision ~= nil and decision.kind or nil
    local key  = decision ~= nil and decision.key or nil
    if me.ghost then
        return
    end

    if task == nil or task == "NOTHING" then
        dog.nothing = dog.nothing or now
        if now - dog.nothing >= M.IDLE then
            actor:violate("nothing")
        end
    else
        dog.nothing = nil
    end

    local crew = {}
    for _, member in ipairs(party.members(snap)) do
        local world = member.world or {}
        table.insert(crew, tostring(world.map) .. ":" .. tostring(world.x) .. ":" .. tostring(world.y))
    end
    local rooted = false
    for _, other in ipairs(actor.session.actors) do
        local window = other.rooted
        rooted = rooted or (window ~= nil and now >= window.from and now < window.to)
    end

    local here = table.concat({ snap.world.map, snap.world.x, snap.world.y, me.hp, me.mp or 0,
        actor.me.stats.kills or 0, actor.me.stats.exp or 0, table.concat(crew, ",") }, "|")
    if dog.here ~= here or rooted then
        dog.here, dog.since = here, now
    elseif now - dog.since >= M.IDLE then
        actor:violate("idle")
    end
    local others = {}
    local others_idle = true
    for _, member in ipairs(party.members(snap)) do
        local world = member.world or {}
        table.insert(others, tostring(world.map) .. "," .. tostring(world.x) .. "," .. tostring(world.y))
        others_idle = others_idle and (member.task == "FIELD" or member.task == "NOTHING" or member.task == nil)
    end
    local spots = table.concat(others, "/")
    local still = dog.others == spots
    dog.others = spots
    if kind == ACTION.HOLD and rooted == false and still and others_idle then
        local node = last_node(path)
        if WORKING[node] ~= true then
            dog.held[node] = (dog.held[node] or 0) + dt
            if now >= M.WAIT_FROM and dog.held[node] >= M.WAIT_SHARE * now then
                actor:violate("waiting:" .. node)
            end
        end
    end

    local members  = party.members(snap)
    local apart    = false
    local plan_now = #members > 0 and party.plan(snap, bound) or nil
    local hunting  = false
    for _, floor in ipairs(plan_now ~= nil and plan_now.floors or {}) do
        hunting = hunting or floor.map == snap.world.map
    end
    for _, member in ipairs(hunting and members or {}) do
        local coming = member.task == "FIELD" and require("brain.policies.ground").floor_at(member.world.map) == nil
        for _, map in ipairs(member.heading or {}) do
            coming = coming or map == snap.world.map
        end
        apart = apart or (member.self.ghost == false and member.world.map ~= snap.world.map and coming == false)
    end
    if apart then
        dog.split = dog.split or now
        if now - dog.split >= M.SPLIT then
            actor:violate("split")
        end
    else
        dog.split = nil
    end

    local spread = false
    for _, member in ipairs(hunting and members or {}) do
        local world = member.world
        spread = spread or (member.self.ghost == false and member.pinned ~= true and world.map == snap.world.map and
            world.x ~= nil and (math.abs(world.x - snap.world.x) > data.sight.x or math.abs(world.y - snap.world.y) > data.sight.y))
    end
    if spread then
        dog.spread = dog.spread or now
        if now - dog.spread >= M.SPLIT then
            actor:violate("spread")
        end
    else
        dog.spread = nil
    end

    local deficit = me.max_hp - me.hp
    local mending = deficit > 0 and recovery.mending(snap, deficit) or nil
    local low     = mending ~= nil and me.hp < me.max_hp / 2 and rest.low(snap, bound)
    local mended  = dog.hp ~= nil and me.hp > dog.hp
    dog.hp        = me.hp
    local crew    = escort.present(snap, bound)
    local tended  = crew ~= nil and crew.heal > 0 and mended
    if counted(dog, "walk_low", low and tended == false and task == "FIELD" and kind == ACTION.MOVE) >= M.REPEAT then
        actor:violate("walk_low_hp")
    end
    local attacking = decision ~= nil and kind == ACTION.ATTACK and key ~= "CHARGE" and key ~= nil and
        battle.burst(snap) > 0 and me.hp <= 2 * battle.burst(snap) and mending ~= nil and
        (decision.choice == nil or decision.choice.kind ~= "HEAL") and task == "DEFEND" and
        (decision.hp == nil or decision.hp <= 0)
    if counted(dog, "attack_low", attacking and require("brain.data.spells").spells[key] ~= nil and
            require("brain.data.spells").spells[key].role ~= "HEAL") >= M.REPEAT then
        actor:violate("attack_low_hp")
    end

    local spot = snap.world.x .. "," .. snap.world.y
    if counted(dog, "escape_stuck", task == "ESCAPE" and dog.escape_at == spot) >= M.STUCK_ESCAPE then
        actor:violate("escape_stuck")
    end
    dog.escape_at = task == "ESCAPE" and spot or nil

    if task == "SUPPLY" or TRIPS[task] or string.find(tostring(path or ""), "errand", 1, true) ~= nil then
        dog.away = (dog.away or 0) + dt
    end
    if now - (dog.away or 0) >= M.KILLS_FROM then
        local kills = 0
        for _, other in ipairs(actor.session.actors) do
            kills = kills + (other.me.stats.kills or 0)
        end
        if kills == 0 then
            actor:violate("no_kills")
        end
    end

    if me.hp < me.max_hp * M.FRAIL_RATIO and battle.burst(snap) > 0 and me.hp <= 2 * battle.burst(snap) then
        dog.frail = (dog.frail or 0) + dt
    end
    if now >= M.FRAIL_FROM and (dog.frail or 0) >= M.FRAIL_SHARE * now then
        actor:violate("frail")
    end

    local least = recovery.least(snap)
    local spare = least ~= nil and (me.mp or 0) >= least
    local cloak = false
    for _, learned in ipairs(snap.spells or {}) do
        cloak = cloak or learned.name == "투명"
    end
    local near = 0
    for _, mob in ipairs(snap.world.mobs or {}) do
        if mob.x ~= nil and math.abs(mob.x - snap.world.x) + math.abs(mob.y - snap.world.y) == 1 then
            near = near + 1
        end
    end
    local target = nil
    for _, mob in ipairs(snap.world.mobs or {}) do
        if decision ~= nil and mob.oid == decision.oid then
            target = mob
        end
    end
    local fueled = target ~= nil and value.fuel(snap, snap, party.members(snap), target) > 0
    local melee  = me.class == "WARRIOR" or (me.class == "ROGUE" and cloak == false) or outswings(snap, bound) or fueled
    if counted(dog, "swings", key == "SWING" and snap.self.cloaked ~= true and near > 0 and melee == false and
            (spare or cloak)) >= M.SWINGS then
        actor:violate("swing_with_spell")
    end

    if (me.class ~= "WARRIOR" or (low and mending.hp > battle.burst(snap))) and least ~= nil and (me.mp or 0) < least and #recovery.doses(snap, "mp") > 0 then
        dog.dry = dog.dry or now
        if now - dog.dry >= M.DRY then
            actor:violate("dry_with_doses")
        end
    else
        dog.dry = nil
    end

    local memory = snap.record.memory
    if #members > 0 and memory.servicing == nil and TRIPS[task] and (kind == ACTION.MOVE or kind == ACTION.HOLD) then
        actor:violate("errand_in_group")
    end
    local stocked = 0
    for _, item in ipairs(snap.bag.items or {}) do
        stocked = stocked + (item.count or 0)
    end
    dog.stocked = dog.stocked or stocked
    if memory.untaught ~= nil and task == "SUPPLY" and kind == ACTION.MOVE and now < M.EARLY and stocked >= dog.stocked then
        actor:violate("supply_early")
    end

    if task == "ESCAPE" and dog.fled_from ~= snap.world.map then
        dog.fled_from = snap.world.map
        local roster  = require("brain.policies.ground").roster_of(snap.world.map)
        local entry   = memory.entry
        local struck  = roster ~= nil and (memory.fled or {})["r:" .. roster] or nil
        if roster ~= nil and type(entry) == "table" and entry.roster == roster and entry.ready == false and entry.at > 0 then
            actor:violate("unready_flight")
        end
        if type(struck) == "table" and struck.power == require("brain.policies.danger").power(snap) and (struck.strikes or 0) > 1 then
            actor:violate("repeat_flight")
        end
    elseif task ~= "ESCAPE" then
        dog.fled_from = nil
    end

    local plan = memory.commit or memory.solo
    if plan ~= nil and plan ~= dog.plan then
        if dog.plan ~= nil then
            table.insert(dog.flips, now)
        end
        dog.plan = plan
        while dog.flips[1] ~= nil and now - dog.flips[1] > M.FLIP_WINDOW do
            table.remove(dog.flips, 1)
        end
        if #dog.flips >= M.FLIPS then
            actor:violate("ground_flip")
        end
    end
end

return M
