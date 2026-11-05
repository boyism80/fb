local decide   = require("brain.decide")
local battle   = require("brain.policies.battle")
local combat   = require("brain.data.combat")
local items    = require("brain.data.items")
local danger   = require("brain.policies.danger")
local escort   = require("brain.policies.escort")
local ground   = require("brain.policies.ground")
local party    = require("brain.policies.party")
local recovery = require("brain.policies.recovery")
local supply   = require("brain.tasks.supply")
local fight    = require("brain.policies.combat")
local value    = require("brain.policies.value")
local facts    = require("sim.facts")
local engine   = require("sim.world.engine")
local json     = require("sim.world.json")
local town     = require("sim.world.town")
local spells   = require("sim.world.spells")
local why      = require("sim.world.why")

local M = {}

local LIMIT = 80
local OIDS  = 1000000

local function copy(value)
    if type(value) ~= "table" then
        return value
    end
    local result = {}
    for key, entry in pairs(value) do
        result[key] = copy(entry)
    end
    return result
end

local Session = {}
Session.__index = Session

local Actor = {}
Actor.__index = Actor

function Session:emit(actor, record)
    record.bot  = actor.base.self.name
    record.t    = self.clock
    record.seed = self.seed
    self.output:write(json.encode(record), "\n")
end

function Session:world(map)
    local world = self.worlds[map]
    if world ~= nil then
        world:advance(self.clock - world.now)
        return world
    end
    local grid = self.mind:grid(map)
    if grid == nil then
        return nil
    end
    world = engine.new({
        map = map,
        grid = grid,
        seed = self.seed * 7919 + map,
        rezen_multiplier = self.options.rezen_multiplier,
        exp_multiplier = self.options.exp_multiplier,
        exp_limit = self.options.exp_limit,
        drops = self.options.drops,
    })
    world.share = function(killer, exp)
        self:share(world, killer, exp)
    end
    world:advance(self.clock)
    self.worlds[map] = world
    return world
end

function Session:group(actor)
    for _, group in ipairs(self.groups) do
        if group.members[actor] then
            return group
        end
    end
    return nil
end

function Session:share(world, killer, exp)
    local owner = self.owners[killer]
    local group = owner ~= nil and self:group(owner) or nil
    if group == nil then
        world:award(killer, exp)
        return
    end
    local nears = {}
    local poet  = false
    for member in pairs(group.members) do
        if member.me.dead == false and member.active == world then
            table.insert(nears, member.me)
            poet = poet or member.base.self.class == "POET"
        end
    end
    if #nears == 0 then
        world:award(killer, exp)
        return
    end
    local bonus = #nears > 1 and poet and exp * (combat.group.bonus.POET or 0) or 0
    local each  = math.floor((exp + bonus) / #nears)
    for _, member in ipairs(nears) do
        world:award(member, each)
    end
end

function Session:roster(actor)
    local found = {}
    for _, other in ipairs(self.actors) do
        if other ~= actor and self.profiles[other] ~= nil then
            table.insert(found, { name = other.base.self.name, profile = self.profiles[other] })
        end
    end
    return found
end

function Session:find(name)
    for _, actor in ipairs(self.actors) do
        if actor.base.self.name == name then
            return actor
        end
    end
    return nil
end

function Session:advance(to)
    for _, world in pairs(self.worlds) do
        if next(world.characters) ~= nil and world.now < to then
            world:advance(to - world.now)
        end
    end
    self.clock = to
end

function Session:run()
    while true do
        local actor = nil
        for _, candidate in ipairs(self.actors) do
            if candidate.finished == false and (actor == nil or candidate.wake < actor.wake) then
                actor = candidate
            end
        end
        if actor == nil then
            return
        end
        self:advance(actor.wake)
        actor.clock = self.clock
        local ok, duration = coroutine.resume(actor.co)
        while ok and type(duration) == "table" and coroutine.status(actor.co) ~= "dead" do
            ok, duration = coroutine.resume(actor.co, table.unpack({ duration.call() }))
        end
        if ok == false then
            error(duration)
        end
        if coroutine.status(actor.co) == "dead" then
            actor.finished = true
        else
            actor.wake = self.clock + math.max(duration or 0, 1)
        end
    end
end

function Actor:emit(record)
    self.session:emit(self, record)
end

function Actor:main(call)
    if coroutine.running() == self.co then
        return coroutine.yield({ call = call })
    end
    return call()
end

function Actor:advance(duration)
    coroutine.yield(duration)
    self.clock = self.session.clock
end

function Actor:enter(map, x, y)
    local world = self:main(function()
        return self.session:world(map)
    end)
    if world == nil then
        return false
    end
    local spots   = {}
    local closest = math.huge
    for row = 0, world.map.height - 1 do
        for column = 0, world.map.width - 1 do
            if world.map:vacant(column, row) then
                local distance = x ~= nil and math.abs(column - x) + math.abs(row - y) or 0
                if distance < closest then
                    spots   = {}
                    closest = distance
                end
                if distance == closest then
                    table.insert(spots, { column, row })
                end
            end
        end
    end
    if #spots == 0 then
        return false
    end
    if self.active ~= nil then
        self.active:remove(self.me)
    end
    local spot    = spots[math.random(1, #spots)]
    self.me.world = world
    world:place(self.me, spot[1], spot[2])
    self.active = world
    self:publish()
    table.insert(self.visits, { map = map, t = self.clock })
    local stats = self.me.stats
    self.stay   = {
        map = map,
        started = self.clock,
        kills = stats.kills,
        exp = self.me.exp,
        taken = stats.taken,
        swings = stats.swings,
        hits = stats.hits
    }
    return true
end

function Actor:leave(result)
    local stay = self.stay
    if stay == nil then
        return
    end
    local stats = self.me.stats
    self:emit({
        kind = "stay",
        result = result,
        map = stay.map,
        level = self.base.self.level,
        class = self.base.self.class,
        crew = self.session:group(self) ~= nil,
        forecast = self.occupied[stay.map],
        actual = {
            time = self.clock - stay.started,
            kills = stats.kills - stay.kills,
            exp = self.me.exp - stay.exp,
            taken = stats.taken - stay.taken,
            swings = stats.swings - stay.swings,
            hits = stats.hits - stay.hits,
            died = self.me.dead,
        },
    })
    self.stay = nil
end

function Actor:visible()
    local found = {}
    local world = self.active
    for _, mob in pairs(world.mobs) do
        if world.map:sight(self.me, mob) then
            local entry = copy(mob.data)
            entry.drops = nil
            entry.oid   = mob.oid
            entry.id    = mob.data.id
            entry.name  = mob.data.name
            entry.x     = mob.x
            entry.y     = mob.y
            entry.ratio = mob.hp / mob.max_hp
            table.insert(found, entry)
        end
    end
    table.sort(found, function(left, right)
        return left.oid < right.oid
    end)
    return found
end

function Actor:piles()
    local found = {}
    for _, pile in pairs(self.active.piles or {}) do
        if self.active.map:sight(self.me, pile) then
            table.insert(found, { oid = pile.oid, x = pile.x, y = pile.y, names = pile.names })
        end
    end
    return found
end

function Actor:pick(oids)
    local picked = false
    for _, oid in ipairs(oids or {}) do
        local pile = (self.active.piles or {})[oid]
        if pile ~= nil and pile.x == self.me.x and pile.y == self.me.y and
            town.gather(self.bag, pile.names[1], pile.count, pile.capacity) then
            self.active.piles[oid] = nil
            picked = true
        end
    end
    return picked
end

function Actor:members()
    local group = self.session:group(self)
    local names = {}
    local found = {}
    if group == nil then
        return names, found, nil
    end
    for member in pairs(group.members) do
        if member ~= self then
            table.insert(names, member.base.self.name)
            local profile = self.session.profiles[member]
            if profile ~= nil then
                table.insert(found, profile)
            end
        end
    end
    table.sort(names)
    return names, found, group.leader.base.self.name
end

function Actor:snapshot()
    local me       = self.me
    local base     = self.base
    local self_    = copy(base.self)
    self_.hp       = me.hp
    self_.mp       = me.mp
    self_.exp      = me.exp
    self_.money    = self.money
    self_.ghost    = me.dead
    self_.struck   = me.struck ~= nil and self.clock - me.struck or nil
    self_.casted   = self.casted ~= nil and self.clock - self.casted or nil
    self_.cloaked  = self:cloaked()
    self_.phydef   = me.phydef
    self_.hit      = me.hit_value
    self_.dam      = me.dam_value
    local failures = {}
    for task, record in pairs(self.failures) do
        failures[task] = {
            failures = record.failures,
            reason = record.reason,
            duration = record.duration,
            age = self.clock - record.at,
            map = record.map,
            level = record.level,
        }
    end
    local names, profiles, leader = self:members()
    return {
        self = self_,
        buffs = self:buffs(),
        spells = self.spells,
        gear = base.gear,
        bag = self.bag,
        world = { map = self.active.map.id, x = me.x, y = me.y, mobs = self:visible(), items = self:piles(), dialog = false, aside = {} },
        record = { failures = failures, memory = self.memory },
        pace = {},
        now = self.clock,
        persona = base.persona or {},
        group = names,
        leader = leader,
        party = profiles,
    }
end

function Actor:cloaked()
    local STATE = self.active.enums.STATE
    local state = self.me.state
    return state == STATE.CLOACK or state == STATE.HALF_CLOACK or state == STATE.ADV_CLOACK
end

function Actor:buffs()
    local names = {}
    for name in pairs(self.me.buffs) do
        table.insert(names, name)
    end
    table.sort(names)
    return names
end

function Actor:signature()
    local parts = { self.me.hp, self.me.mp }
    for _, mob in pairs(self.active.mobs) do
        if self.active.map:sight(self.me, mob) then
            table.insert(parts, string.format("%d:%d:%d:%d", mob.oid, mob.x, mob.y, mob.hp))
        end
    end
    for character in pairs(self.active.characters) do
        if character ~= self.me then
            table.insert(parts, string.format("c%d:%d:%d:%d:%d", character.oid, character.x, character.y,
                character.hp, character.mp or 0))
        end
    end
    local _, profiles = self:members()
    for _, profile in ipairs(profiles) do
        table.insert(parts, string.format("p%s:%d:%d:%d", tostring(profile.self.name), profile.world.map,
            profile.world.x or 0, profile.world.y or 0))
    end
    table.sort(parts, function(left, right)
        return tostring(left) < tostring(right)
    end)
    return table.concat(parts, "|", 1, #parts)
end

function Actor:wait(revisit)
    local before   = self:signature()
    local deadline = revisit ~= nil and self.clock + math.max(revisit, 1) or self.ends
    while self.clock < math.min(deadline, self.ends) and self.me.dead == false do
        self:advance(100)
        if self:signature() ~= before then
            return
        end
    end
end

function Actor:open(oid, action)
    local fight = self.current
    if fight ~= nil and fight.oid ~= oid then
        self:close("SWITCHED")
        fight = nil
    end
    if fight == nil then
        local stats  = self.me.stats
        fight        = {
            oid = oid,
            mob = action.mob,
            id = action.mob_id,
            task = action.task,
            map = self.active.map.id,
            started = self.clock,
            hp = self.me.hp,
            mp = self.me.mp,
            exp = self.me.exp,
            low = self.me.hp,
            forecast = action.forecast,
            uses = {},
            events = {},
            steps = {},
            swings = stats.swings,
            hits = stats.hits,
            taken = stats.taken,
            dealt = stats.dealt,
            walk = 0,
        }
        self.current = fight
    end
end

function Actor:fight(oid, action, event)
    local fight = self.current
    if fight == nil or fight.oid ~= oid then
        return
    end
    local key           = action.key or "SWING"
    fight.uses[key]     = (fight.uses[key] or 0) + 1
    fight.events[event] = (fight.events[event] or 0) + 1
    fight.low           = math.min(fight.low, self.me.hp)
    if #fight.steps < LIMIT then
        local near = 0
        for _, mob in pairs(self.active.mobs) do
            if math.abs(mob.x - self.me.x) + math.abs(mob.y - self.me.y) == 1 then
                near = near + 1
            end
        end
        table.insert(fight.steps, {
            at = self.clock - fight.started,
            key = key,
            event = event,
            hp = self.me.hp,
            mp = self.me.mp,
            near = near
        })
    end
    if event == "KILLED" then
        self:close("KILLED")
    end
end

function Actor:close(result)
    local fight = self.current
    if fight == nil then
        return
    end
    self.current = nil
    local stats  = self.me.stats
    self:emit({
        kind = "fight",
        source = "world",
        result = result,
        task = fight.task,
        oid = fight.oid,
        mob = fight.mob,
        id = fight.id,
        map = fight.map,
        level = self.base.self.level,
        class = self.base.self.class,
        crew = self.session:group(self) ~= nil,
        forecast = fight.forecast,
        actual = {
            time = self.clock - fight.started,
            hp = self.me.hp - fight.hp,
            mp = self.me.mp - fight.mp,
            exp = self.me.exp - fight.exp,
            low = fight.low,
            uses = fight.uses,
            events = fight.events,
            steps = fight.steps,
            swings = stats.swings - fight.swings,
            hits = stats.hits - fight.hits,
            taken = stats.taken - fight.taken,
            walk = fight.walk,
            dealt = stats.dealt - fight.dealt,
        },
    })
end

function Actor:face(target)
    local DIRECTION = self.active.enums.DIRECTION
    local dx, dy    = target.x - self.me.x, target.y - self.me.y
    if dx == 0 and dy == 0 then
        return
    end
    self.me.facing = dx < 0 and DIRECTION.LEFT or dx > 0 and DIRECTION.RIGHT or dy < 0 and DIRECTION.TOP or
        DIRECTION.BOTTOM
end

function Actor:sighted(oid)
    local world = self.active
    local mob   = world.mobs[oid]
    if mob == nil then
        return nil, world.killed[oid] and "KILLED" or "LOST"
    elseif world.map:sight(self.me, mob) == false then
        return nil, "LOST"
    end
    return mob
end

function Actor:engage(oid)
    local world      = self.active
    local start      = self.me.hp
    local mob, ended = self:sighted(oid)
    if mob == nil then
        return ended
    end
    if math.abs(mob.x - self.me.x) + math.abs(mob.y - self.me.y) ~= 1 then
        local goal = { x = mob.x, y = mob.y }
        local path = world.map:route(self.me, function(x, y)
            return math.abs(x - goal.x) + math.abs(y - goal.y) == 1
        end)
        if path == nil or #path == 0 then
            local open = world.map:route(self.me, function(x, y)
                return math.abs(x - goal.x) + math.abs(y - goal.y) == 1
            end, true)
            self.blocked = { walls = open == nil, x = goal.x, y = goal.y, me = { self.me.x, self.me.y } }
            return "UNREACHABLE"
        end
        for _, direction in ipairs(path) do
            if world:move(self.me, direction) == false then
                break
            end
            self:advance(self.pace.step)
            if self.current ~= nil then
                self.current.walk = self.current.walk + self.pace.step
            end
            if self.me.dead then
                break
            end
        end
        mob, ended = self:sighted(oid)
        if mob == nil then
            return ended
        end
        return self.me.hp < start and "STRUCK" or "APPROACHED"
    end
    self:face(mob)
    world:swing(self.me)
    self:advance(self.pace.swing)
    mob, ended = self:sighted(oid)
    if mob == nil then
        return ended
    end
    return self.me.hp < start and "STRUCK" or "DONE"
end

function Actor:consume(name, count)
    for _ = 1, count or 1 do
        local entry = nil
        for _, item in ipairs(self.bag.items) do
            if item.name == name and item.count > 0 then
                entry = item
            end
        end
        local effect = items[name]
        if entry == nil or effect == nil then
            return false
        end
        entry.count     = entry.count - 1
        local snap      = { self = { max_hp = self.me.max_hp, max_mp = self.me.max_mp } }
        self.me.hp      = math.min(self.me.max_hp, self.me.hp + math.floor(recovery.restored(snap, effect, "hp")))
        self.me.mp      = math.min(self.me.max_mp, self.me.mp + math.floor(recovery.restored(snap, effect, "mp")))
        self.used[name] = (self.used[name] or 0) + 1
    end
    return true
end

function Actor:equip(name)
    local entry = nil
    for index, item in ipairs(self.bag.items) do
        if item.name == name and item.count > 0 then
            entry = index
        end
    end
    local model = self:main(function()
        return self.mind:item(name)
    end)
    if entry == nil or model == nil then
        return false
    end
    local item   = table.remove(self.bag.items, entry)
    local slots  = {}
    local done   = false
    local was    = nil
    local vacant = false
    for _, slot in ipairs(self.base.gear or {}) do
        vacant = vacant or (slot.type == model.type and #slot.names == 0)
    end
    for _, slot in ipairs(self.base.gear or {}) do
        if slot.type == model.type and done == false and (vacant == false or #slot.names == 0) then
            was = slot.names[1]
            for _, worn in ipairs(slot.names) do
                table.insert(self.bag.items, { slot = #self.bag.items + 1, name = worn, count = 1, type = slot.type })
            end
            table.insert(slots, { type = slot.type, names = { item.name } })
            done = true
        else
            table.insert(slots, slot)
        end
    end
    if done == false then
        table.insert(slots, { type = model.type, names = { item.name } })
    end
    local base = {}
    for key, value in pairs(self.base) do
        base[key] = value
    end
    base.gear = slots
    local old = was ~= nil and self:main(function()
        return self.mind:item(was)
    end) or {}
    local function delta(key)
        return (model[key] or 0) - (old[key] or 0)
    end
    base.self         = copy(base.self)
    base.self.max_hp  = base.self.max_hp + delta("hp")
    self.me.max_hp    = self.me.max_hp + delta("hp")
    self.me.phydef    = (self.me.phydef or 0) + delta("defense")
    self.me.hit_value = (self.me.hit_value or 0) + delta("hit")
    self.me.dam_value = (self.me.dam_value or 0) + delta("bonus")
    self.base         = base
    if model.type == "WEAPON" then
        self.me.arm = engine.arm(self.active, model)
    end
    return true
end

function Actor:spell(slot)
    for _, known in ipairs(self.spells) do
        if known.slot == slot then
            return known.name
        end
    end
    return nil
end

function Actor:cast(slot, oid, myself)
    local name = self:spell(slot)
    if name == nil then
        return false
    end
    local world  = self.active
    local target = self.me
    if myself ~= true and oid ~= nil then
        target = world:find(oid)
    end
    if target ~= nil and target ~= self.me then
        self:face(target)
    end
    local ok, reason = spells.cast(world, self.me.facade, name, target ~= nil and target.facade or nil)
    self.casted      = self.clock
    self.casts[name] = (self.casts[name] or 0) + 1
    self:advance(self.pace.latency)
    return ok, reason
end

function Actor:attack(action)
    local maneuver = action.maneuver or {}
    local event    = nil
    local x, y     = self.me.x, self.me.y
    self:open(action.oid, action)
    if maneuver.kind == MANEUVER.USE then
        event = self:consume(maneuver.item, 1) and "DONE" or "REFUSED"
    elseif maneuver.kind == MANEUVER.CAST then
        local mp       = self.me.mp
        local ok       = self:cast(maneuver.slot, maneuver.self and nil or action.oid, maneuver.self)
        local _, ended = self:sighted(action.oid)
        if ended == "KILLED" then
            event = "KILLED"
        elseif ok and (self.me.mp < mp or (maneuver.cost or 0) == 0) then
            event = "DONE"
        else
            event = "REFUSED"
        end
    else
        event = self:engage(action.oid)
    end
    self:fight(action.oid, action, event)
    local visible = {}
    for _, mob in ipairs(self:visible()) do
        visible[tostring(mob.oid)] = mob
    end
    self.memory.battle, self.memory.unreachable = battle.settle(self.memory.battle, self.memory.unreachable,
        action.oid, self.active.map.id, action, { type = event }, self.clock, visible, x ~= self.me.x or y ~= self.me.y)
    return event == "KILLED" or event == "UNREACHABLE" or battle.going(event), event
end

function Actor:walk(dest)
    local world = self.active
    local range = dest.range or 0
    local path  = world.map:route(self.me, function(x, y)
        return math.abs(x - dest.x) + math.abs(y - dest.y) <= range
    end)
    if path == nil then
        return false, "NO_ROUTE"
    end
    local hp = self.me.hp
    for _, direction in ipairs(path) do
        if world:move(self.me, direction) == false then
            return false, "BLOCKED"
        end
        self:advance(self.pace.step)
        self:publish()
        if self.me.hp < hp or self.me.dead then
            return true, "ATTACKED"
        end
    end
    return true, "ARRIVED"
end

function Actor:publish()
    local profile = self.session.profiles[self]
    if profile ~= nil and self.active ~= nil then
        profile.world = { map = self.active.map.id, x = self.me.x, y = self.me.y }
    end
end

function Actor:travel(dest)
    local here = self.active.map.id
    if dest.map == here and dest.x ~= nil then
        return self:walk(dest)
    end
    local maps = dest.maps or { dest.map }
    for _, map in ipairs(maps) do
        if map == here then
            return true, "ARRIVED"
        end
    end
    local snap = self:snapshot()
    local jump = value.leap(snap, { maps = maps })
    if jump and type(jump.land) == "number" then
        local landing = self:main(function()
            return self.mind:route(nil, { maps = { jump.land } }, { perilous = {}, shunned = {} },
                { map = here, x = self.me.x, y = self.me.y, level = self.base.self.level })
        end)
        if landing ~= nil and landing.cost * self.pace.step > fight.cast(snap) then
            if jump.item ~= nil then
                for _, item in ipairs(self.bag.items) do
                    if item.name == jump.item.name and item.count > 0 then
                        item.count = item.count - 1
                        break
                    end
                end
            else
                self.me.mp = math.max(0, self.me.mp - (jump.spell.mp or 0))
            end
            self:close("LEFT")
            self:leave("MOVED")
            self.active:remove(self.me)
            self.transit = true
            self:advance(fight.cast(snap))
            self.transit = false
            if self:enter(landing.dest, landing.x, landing.y) == false then
                self.departed = landing.dest
                return false, "NO_GRID"
            end
            return true, "ARRIVED"
        end
    end
    local route = self:main(function()
        return self.mind:route(nil, { maps = maps }, { perilous = {}, shunned = {} },
            { map = here, x = self.me.x, y = self.me.y, level = self.base.self.level })
    end)
    if route == nil then
        return false, "NO_ROUTE"
    end
    local exit  = (route.path or {})[1]
    local spent = 0
    if exit ~= nil and exit.x ~= nil then
        spent = (math.abs(exit.x - self.me.x) + math.abs(exit.y - self.me.y)) * self.pace.step
        local _, reason = self:walk({ map = here, x = exit.x, y = exit.y, range = 0 })
        if self.me.dead then
            return false, "DIED"
        end
        if reason == "ATTACKED" then
            return true, "ATTACKED"
        end
    end
    self:close("LEFT")
    self:leave("MOVED")
    self.active:remove(self.me)
    local destination, x, y = route.dest, route.x, route.y
    if exit ~= nil and exit.dest ~= nil and exit.ax ~= nil then
        destination, x, y = exit.dest, exit.ax, exit.ay
        self.transit = true
        local gap = self.options.transfer or 0
        self:advance(gap)
        engine.rest(self.me, gap)
        self.transit = false
    else
        self.transit = true
        local remaining = math.max(0, route.cost * self.pace.step - spent)
        self:advance(remaining)
        engine.rest(self.me, remaining)
        self.transit = false
    end
    if self:enter(destination, x, y) == false then
        self.departed = destination
        return false, "NO_GRID"
    end
    return true, "ARRIVED"
end

function Actor:arrival(map)
    local base  = self.base
    local from  = { map = base.world.map, x = base.world.x, y = base.world.y, level = base.self.level }
    local route = self.mind:route(nil, { maps = { map } }, { perilous = {}, shunned = {} }, from)
    if route == nil or route.dest ~= map then
        return nil
    end
    return { x = route.x, y = route.y }
end

function Actor:invite(name)
    local other = self.session:find(name)
    if other == nil or other.finished or other.base.self.grouping ~= true or self.session:group(other) ~= nil then
        return false, "REFUSED"
    end
    local group = self.session:group(self)
    if group == nil then
        group = { leader = self, members = { [self] = true }, formed = self.clock }
        table.insert(self.session.groups, group)
    end
    group.members[other] = true
    self:emit({ kind = "group", event = "JOIN", member = name, leader = group.leader.base.self.name })
    return true
end

function Actor:part(action)
    local group = self.session:group(self)
    if group == nil then
        return false, "REFUSED"
    end
    group.members[self] = nil
    local left = {}
    for member in pairs(group.members) do
        table.insert(left, member)
    end
    if #left < 2 then
        for index, entry in ipairs(self.session.groups) do
            if entry == group then
                table.remove(self.session.groups, index)
                break
            end
        end
    elseif group.leader == self then
        group.leader = left[1]
    end
    self.memory.left = { names = action.names, at = self.clock, waited = action.waited }
    self:emit({ kind = "group", event = "LEAVE", names = action.names, waited = action.waited })
    return true
end

function Actor:perform(action)
    if action.occupy ~= nil and self.occupied[action.occupy.map] == nil then
        self.occupied[action.occupy.map] = {
            kills = action.occupy.rate,
            exp = action.occupy.exp,
            deaths = action.occupy.deaths,
            margin = action.occupy.margin,
            wear = action.occupy.wear,
            extra = action.occupy.extra,
            name = action.occupy.name,
            crew = action.occupy.crew,
        }
        self:emit({
            kind = "ground",
            source = "world",
            map = action.occupy.map,
            level = self.base.self.level,
            class = self.base.self.class,
            exp = self.me.exp,
            forecast = self.occupied[action.occupy.map],
        })
    end
    if action.kind == ACTION.ATTACK then
        return self:attack(action)
    elseif action.kind == ACTION.MOVE then
        return self:travel(action.dest)
    elseif action.kind == ACTION.USE then
        return self:consume(action.item, action.count), "NO_ITEM"
    elseif action.kind == ACTION.CAST then
        local hp, mp = self.me.hp, self.me.mp
        local ok, reason = self:cast(action.slot, action.oid, action.oid == nil)
        if ok == false then
            return false, reason or "NO_SPELL"
        elseif action.buff ~= nil then
            return self.me.buffs[action.buff] ~= nil, "UNCHANGED"
        end
        return self.me.hp ~= hp or self.me.mp ~= mp, "UNCHANGED"
    elseif action.kind == ACTION.EQUIP then
        return self:equip(action.item), "REFUSED"
    elseif action.kind == ACTION.PICK then
        return self:pick(action.oids), "UNCHANGED"
    elseif action.kind == ACTION.SAY then
        return town.say(self, action)
    elseif action.kind == ACTION.TALK then
        return town.talk(self, action)
    elseif action.kind == ACTION.INVITE then
        return self:invite(action.name)
    elseif action.kind == ACTION.LEAVE then
        return self:part(action)
    elseif action.kind == ACTION.FORGET then
        if action.key ~= nil then
            self.memory[action.key] = nil
        end
        return true
    end
    local name = tostring(action.task) .. ":" .. tostring(action.kind)
    self.unsupported[name] = (self.unsupported[name] or 0) + 1
    return false, "UNSUPPORTED"
end

function Actor:settle(task, done, reason, elapsed)
    if done then
        self.failures[task] = nil
        return
    end
    local record        = self.failures[task] or { failures = 0 }
    local grown         = record.failures > 0 and record.level ~= self.base.self.level
    record.failures     = grown and 1 or record.failures + 1
    record.reason       = reason
    record.at           = self.clock
    record.duration     = math.max(elapsed, 100)
    record.map          = self.active.map.id
    record.level        = self.base.self.level
    self.failures[task] = record
end

function Actor:think()
    local snap  = self:snapshot()
    local bound = facts.bind(self.mind, snap, {
        roster = self.session:roster(self),
        ally = function(name)
            local other = self.session:find(name)
            if other == nil or other.transit or other.active ~= self.active or other.me.dead or
                self.active.map:sight(self.me, other.me) == false then
                return nil
            end
            return { oid = other.me.oid, x = other.me.x, y = other.me.y, percent = other.me.hp * 100 // other.me.max_hp }
        end
    })
    local apart = #(snap.party or {}) > 0 and escort.complete(snap, bound) == false
    if apart and self.memory.apart == nil then
        self.memory.apart = snap.now
    elseif apart == false and self.memory.apart ~= nil then
        self.memory.apart = nil
    end
    local threat = snap.self.ghost == false and danger.fatal(snap, bound) or nil
    if threat ~= nil then
        local perils = self.memory.perils or {}
        perils[tostring(snap.world.map)] = { mob = threat.id, stats = threat, at = snap.now }
        self.memory.perils = perils
    end
    local changed, commit, swept = decide.commit(snap, bound)
    if changed then
        self.memory.commit = commit
        self.memory.swept  = swept
    end
    local decision, revisit     = decide.choose(snap, bound, nil, nil)
    self.session.profiles[self] = party.profile(snap, bound, decision)
    if self.options.acts then
        if decision ~= nil and decision.task == "ESCAPE" and decision.cause ~= nil then
            self:emit({ kind = "why", cause = decision.cause, map = snap.world.map, detail = why.escape(snap, bound,
                decision) })
        end
        local plan = party.plan(snap, bound)
        local name = plan ~= nil and plan.name or ""
        if name ~= self.reported then
            self.reported = name
            self:emit({ kind = "plan", map = snap.world.map, detail = why.plan(snap, bound) })
            self:emit({ kind = "survey", map = snap.world.map, detail = why.survey(snap, bound) })
        end
    end
    local fled = decide.flee(snap, decision)
    if fled ~= nil then
        self.memory.fled = fled
    end
    local swept = decide.sweep(snap, decision)
    if swept ~= nil then
        self.memory.swept = swept
    end
    local warded = decide.warded(snap, decision)
    if warded ~= nil then
        self.memory.warded = warded
    end
    local fleeing = decide.fleeing(snap, decision)
    if fleeing ~= nil then
        self.memory.fleeing = fleeing
    end
    return decision, revisit
end

function Actor:run()
    while self.clock < self.ends and self.me.dead == false and self.departed == nil do
        local action, revisit = self:main(function()
            return self:think()
        end)
        self.decisions        = self.decisions + 1
        if action ~= nil and action.task ~= nil then
            self.tasks[action.task] = (self.tasks[action.task] or 0) + 1
        end
        if action == nil or action.kind == ACTION.HOLD or action.kind == ACTION.TASK or action.task == nil then
            local started = self.clock
            local why     = action ~= nil and tostring(action.task) or "NOTHING"
            self:wait(action ~= nil and action.revisit or revisit)
            self.idle[why] = (self.idle[why] or 0) + self.clock - started
            if self.options.acts then
                self:emit({
                    kind = "act",
                    task = why,
                    action = action ~= nil and action.kind or nil,
                    wait = self.clock - started,
                    started = started,
                    map = self.active.map.id,
                    revisit = revisit
                })
            end
        else
            local started      = self.clock
            local done, reason = self:perform(action)
            if self.current ~= nil and self.me.dead then
                self:close("DIED")
            end
            if done and action.clear ~= nil then
                self.memory[action.clear] = nil
            end
            self:settle(action.task, done, reason, self.clock - started)
            if self.options.acts then
                self:emit({
                    kind = "act",
                    task = action.task,
                    action = action.kind,
                    done = done,
                    reason = reason,
                    started = started,
                    map = self.active.map.id,
                    x = self.me.x,
                    y = self.me.y,
                    hp = self.me.hp,
                    mp = self.me.mp,
                    money = self.money,
                    text = action.text,
                    dest = action.dest,
                    item = action.item,
                    key = action.key,
                    mob = action.mob,
                    oid = action.oid,
                    name = action.name,
                    cause = action.cause
                })
            end
            if done == false then
                self:emit({
                    kind = "action",
                    source = "world",
                    task = action.task,
                    action = action.kind,
                    reason = reason,
                    map = self.active.map.id,
                    failures = self.failures[action.task].failures,
                    mob = action.mob,
                    blocked = reason == "UNREACHABLE" and self.blocked or nil
                })
            end
            if self.clock == started then
                self:advance(self.pace.latency)
            end
            if done == false then
                self:wait(action.revisit or revisit)
            end
        end
    end
    local result = self.me.dead and "DIED" or self.departed ~= nil and "LEFT" or "END"
    self:close(result)
    self:leave(result)
    if self.active ~= nil then
        self.active:remove(self.me)
    end
    local group = self.session:group(self)
    self:emit({
        kind = "episode",
        source = "world",
        level = self.base.self.level,
        class = self.base.self.class,
        time = self.clock,
        died = self.me.dead,
        grouped = group ~= nil,
        visits = self.visits,
        tasks = self.tasks,
        idle = self.idle,
        casts = self.casts,
        used = self.used,
        unsupported = self.unsupported,
        exp = self.me.exp - (self.base.self.exp or 0),
        money = self.money - (self.base.self.money or 0),
        stats = self.me.stats,
    })
end

function M.prepare(profile, options)
    local base = copy(profile)
    if options.spells == false then
        base.spells = {}
        base.buffs  = {}
    end
    base.record      = base.record or { failures = {}, memory = {} }
    base.pace        = base.pace or {}
    base.world       = base.world or {}
    base.world.mobs  = {}
    base.world.items = {}
    return base
end

function M.ready(base, bound)
    base.self.hp = base.self.max_hp
    base.self.mp = base.self.max_mp
    base.bag     = base.bag or { items = {}, used = 0, capacity = 0 }
    for _ = 1, 8 do
        local choice, count = supply.order(base, bound)
        local cost          = choice ~= nil and (count or 0) * choice.price or 0
        if choice == nil or (count or 0) <= 0 or cost > (base.self.money or 0) then
            return
        end
        base.self.money = base.self.money - cost
        local found     = nil
        for _, item in ipairs(base.bag.items) do
            if item.name == choice.name then
                found = item
            end
        end
        if found == nil then
            found = { slot = #base.bag.items + 1, name = choice.name, count = 0, type = "CONSUME" }
            table.insert(base.bag.items, found)
            base.bag.used = base.bag.used + 1
        end
        found.count = found.count + count
    end
end

function M.session(mind, options)
    return setmetatable({
        mind = mind,
        options = options,
        seed = options.seed or 1,
        output = options.output,
        clock = 0,
        worlds = {},
        actors = {},
        owners = {},
        groups = {},
        profiles = {},
    }, Session)
end

function Session:join(profile, overrides, at)
    local options = self.options
    local base    = M.prepare(profile, options)
    for key, value in pairs(overrides or {}) do
        base.self[key] = value
    end
    local bound = facts.bind(self.mind, base)
    if options.ready then
        M.ready(base, bound)
    end
    local start = at or (options.start and { map = base.world.map, x = base.world.x, y = base.world.y }) or nil
    local map   = start ~= nil and start.map or nil
    if map == nil then
        local best = ground.choose(base, bound, ground.mode(base))
        map        = best ~= nil and best.floors ~= nil and best.floors[1] ~= nil and best.floors[1].map or nil
        if options.place == "origin" then
            map = base.world.map
        end
    end
    if map == nil then
        return nil, "NO_GROUND"
    end
    local first = self:world(map)
    if first == nil then
        return nil, "NO_GRID"
    end
    local actor  = setmetatable({
        session = self,
        mind = self.mind,
        base = base,
        options = options,
        clock = self.clock,
        ends = options.duration,
        memory = {},
        failures = {},
        occupied = {},
        unsupported = {},
        tasks = {},
        idle = {},
        visits = {},
        casts = {},
        used = {},
        decisions = 0,
        wake = self.clock,
        finished = false,
        bag = copy(base.bag or { items = {}, used = 0, capacity = 0 }),
        money = base.self.money or 0,
        spells = copy(base.spells or {}),
        pace = { swing = options.swing or combat.swing, step = options.step or combat.walk, latency = options.latency },
    }, Actor)
    actor.me     = engine.character(first, base, bound)
    actor.me.oid = OIDS + #self.actors + 1
    local origin = (start ~= nil and start.x ~= nil and start) or actor:arrival(map) or
        { x = base.world.x, y = base.world.y }
    if actor:enter(map, origin.x, origin.y) == false then
        return nil, "NO_SPOT"
    end
    self.owners[actor.me] = actor
    table.insert(self.actors, actor)
    actor.co = coroutine.create(function()
        actor:run()
    end)
    return actor
end

function Actor:summary()
    local base  = self.base
    local stats = self.me.stats
    return {
        name = base.self.name,
        class = base.self.class,
        level = base.self.level,
        map = self.visits[1] and self.visits[1].map or 0,
        time = self.clock,
        died = self.me.dead,
        departed = self.departed,
        kills = stats.kills,
        exp = self.me.exp - (base.self.exp or 0),
        swings = stats.swings,
        hits = stats.hits,
        taken = stats.taken,
        dealt = stats.dealt,
        decisions = self.decisions,
        tasks = self.tasks,
        idle = self.idle,
        casts = self.casts,
        used = self.used,
        grouped = self.session:group(self) ~= nil,
        unsupported = self.unsupported,
    }
end

function M.run(mind, profile, options)
    local session      = M.session(mind, options)
    local actor, cause = session:join(profile)
    if actor == nil then
        return { skipped = cause }
    end
    session:run()
    return actor:summary()
end

function M.team(mind, profiles, options, ground)
    local session = M.session(mind, options)
    local actors  = {}
    local at      = nil
    if ground ~= nil and ground.map ~= nil then
        local home = profiles[1].profile.world
        at = session:world(home.map) ~= nil and { map = home.map, x = home.x, y = home.y } or { map = ground.map }
    end
    for _, entry in ipairs(profiles) do
        local actor, cause = session:join(entry.profile, entry.overrides, at)
        if actor ~= nil and options.together and (at == nil or at.x == nil) then
            at = { map = actor.active.map.id, x = actor.me.x, y = actor.me.y }
        end
        if actor ~= nil and ground ~= nil and ground.bundle ~= nil then
            actor.memory.commit = ground.bundle
        end
        if actor == nil then
            return { skipped = cause }
        end
        table.insert(actors, actor)
    end
    if ground ~= nil and ground.form ~= true and #actors > 1 then
        local group = { leader = actors[1], members = {}, formed = session.clock }
        for _, actor in ipairs(actors) do
            group.members[actor] = true
        end
        table.insert(session.groups, group)
    end
    session:run()
    local results = {}
    for _, actor in ipairs(actors) do
        table.insert(results, actor:summary())
    end
    return results, session
end

return M
