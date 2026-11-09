local growth  = require("brain.data.growth")
local hunting = require("brain.data.hunting")
local mapping = require("sim.world.map")
local objects = require("sim.world.objects")
local runtime = require("sim.world.runtime")
local spells  = require("sim.world.spells")

local M = {}

local RULES = {
    tick = 100,
    lock = 5000,
    memory = 10000,
    flee = 0.2,
    recovery = { hp = { every = 10000, rate = 0.005 }, mp = { every = 25000, rate = 0.02 } },
    limit = 3.3,
}

local World = {}
World.__index = World

local function truncate(value)
    return value < 0 and math.ceil(value) or math.floor(value)
end

local function weapon(item, enums)
    if item == nil or item.small == nil then
        return nil
    end
    local kinds = { [0] = "NORMAL", [1] = "SPEAR", [2] = "BOW", [3] = "FAN" }
    local kind  = kinds[(item.dress or 0) // 10000] or "UNKNOWN"
    return { weapon = enums.WEAPON_TYPE[kind], small = item.small, large = item.large, sound = item.sound or 0 }
end

function M.new(options)
    local enums     = runtime.enums()
    local world     = setmetatable({
        enums = enums,
        now = 0,
        options = options,
        mobs = {},
        killed = {},
        rezens = {},
        births = {},
        characters = {},
        serial = 0,
        stats = { swings = 0, hits = 0, misses = 0, struck = 0, taken = 0, kills = 0, exp = 0 },
    }, World)
    world.map       = mapping.new(options.map, options.grid, enums)
    world.surface   = nil
    world.runtime   = runtime.new({
        siege_active = function()
            return false
        end,
    })
    world.on_attack = world.runtime.load("interaction.lua").on_attack
    math.randomseed(options.seed or 1)
    for _, spawn in ipairs(world.map.spawns) do
        local data = hunting.mobs[spawn.mob]
        if data ~= nil then
            local rezen = { spawn = spawn, data = data, alive = 0, due = nil }
            table.insert(world.rezens, rezen)
            world:populate(rezen, math.ceil(spawn.count * (options.sparse or 1)))
        end
    end
    return world
end

function World:random(low, high)
    return math.random(low, high)
end

function World:populate(rezen, count)
    local spawn            = rezen.spawn
    local vacant, walkable = {}, {}
    local width            = math.max(spawn.x1 - spawn.x0, 1)
    local height           = math.max(spawn.y1 - spawn.y0, 1)
    for dy = 0, height - 1 do
        for dx = 0, width - 1 do
            local x, y = spawn.x0 + dx, spawn.y0 + dy
            if self.map:open(x, y) then
                table.insert(walkable, { x, y })
                if self.map:vacant(x, y) then
                    table.insert(vacant, { x, y })
                end
            end
        end
    end
    for _ = 1, count do
        local pool = #vacant > 0 and vacant or walkable
        if #pool == 0 then
            return
        end
        local index = self:random(1, #pool)
        local point = table.remove(pool, index)
        self:spawn(rezen, point[1], point[2])
    end
end

function World:spawn(rezen, x, y)
    self.serial = self.serial + 1
    local data  = rezen.data
    local mob   = objects.life(self, {
        type = self.enums.OBJECT_TYPE.MOB,
        oid = self.serial,
        label = data.name,
        data = data,
        x = x,
        y = y,
        facing = self.enums.DIRECTION.BOTTOM,
        hp = data.hp,
        max_hp = data.hp,
        phydef = data.defense or 0,
        magdef = data.ward or 0,
        resists = data.resists or {},
        rezen = rezen,
        acted = self.now,
        damaged = {},
        lock = nil,
    })
    rezen.alive = rezen.alive + 1
    self.map:enter(mob)
    self.mobs[mob.oid] = mob
    table.insert(self.births, mob)
    return mob
end

function M.stats(snap)
    local table = (growth[snap.self.class] or {})[snap.self.level] or {}
    return { str = table.str or 0, int = table.int or 0, dex = table.dex or 0 }
end

function M.arm(world, item)
    return weapon(item, world.enums)
end

function M.character(world, snap, query)
    local enums = world.enums
    local item  = nil
    for _, slot in ipairs(snap.gear or {}) do
        if slot.type == "WEAPON" and slot.names[1] ~= nil then
            item = query.item(slot.names[1])
        end
    end
    world.serial = world.serial + 1
    local me     = objects.life(world, {
        type = enums.OBJECT_TYPE.CHARACTER,
        oid = world.serial,
        label = snap.self.name,
        class = snap.self.class,
        level = snap.self.level,
        exp = snap.self.exp or 0,
        hp = snap.self.hp,
        max_hp = snap.self.max_hp,
        mp = snap.self.mp or 0,
        max_mp = snap.self.max_mp or 0,
        phydef = snap.self.phydef or 0,
        hit_value = snap.self.hit or 0,
        dam_value = snap.self.dam or 0,
        regen = snap.self.regen or 0,
        stats = M.stats(snap),
        promotion = snap.self.promotion or 0,
        resists = {},
        arm = weapon(item, enums),
        facing = enums.DIRECTION.BOTTOM,
        recovery = {},
        stats = { swings = 0, hits = 0, misses = 0, struck = 0, taken = 0, kills = 0, exp = 0, dealt = 0 },
    })
    return me
end

function World:place(object, x, y)
    object.x, object.y = x, y
    self.map:enter(object)
    self.characters[object] = true
end

function World:remove(object)
    self.map:leave(object)
    self.characters[object] = nil
end

function World:nearest(mob, adjacent)
    local best, distance = nil, nil
    for character in pairs(self.characters) do
        if self:visible(mob, character) and (adjacent == false or self:adjacent(mob, character) ~= nil) then
            local gap = math.abs(character.x - mob.x) + math.abs(character.y - mob.y)
            if distance == nil or gap < distance or (gap == distance and character.oid < best.oid) then
                best, distance = character, gap
            end
        end
    end
    return best
end

function World:roll(object, size)
    if object:is(self.enums.OBJECT_TYPE.MOB) then
        local data = object.data
        if data.damage <= (data.least or data.damage) then
            return data.damage
        end
        return (data.least or 0) + self:random(0, data.damage - (data.least or 0))
    end
    if object.arm == nil then
        return self:random(1, 5)
    end
    local range = size == self.enums.MOB_SIZE.SMALL and object.arm.small or object.arm.large
    local low   = math.max(1, range.min)
    local high  = math.max(low, range.max)
    return self:random(low, high)
end

function World:calculate(attacker, value, target, critical, rate, physical)
    local def     = physical and target.phydef or -math.max(-127, math.min(127, target.magdef or 0))
    local n       = truncate((100 - def) / 10)
    local percent = -125 + (n * (2 * 14.75 - (n - 1) / 2.0)) / 2.0
    local damage  = value - truncate(percent * (value / 100.0))
    local resist  = target.resists[physical and "PHYSICAL" or "MAGIC"] or 0
    damage        = math.floor(damage * (1.0 - resist))
    if physical and target.facing == attacker.facing then
        rate = rate * 2.0
    end
    if critical then
        rate = rate * 2.0
    end
    rate = rate / (target.derate / 1000.0)
    return math.max(0, math.floor(damage * rate))
end

function World:damage(attacker, target, value, options)
    if target.dead then
        return
    end
    local physical = options.physical ~= false
    local final    = options.fixed and value or
        self:calculate(attacker, value, target, options.critical == true, options.rate or 1.0, physical)
    local dealt    = math.min(final, target.hp)
    target.hp      = target.hp - dealt
    if target:is(self.enums.OBJECT_TYPE.MOB) then
        self:remember(target, attacker)
        if attacker ~= nil and attacker.stats ~= nil then
            attacker.stats.hits  = attacker.stats.hits + 1
            attacker.stats.dealt = attacker.stats.dealt + dealt
        end
        if target.hp == 0 then
            self:kill(target, attacker)
        end
    else
        target.stats.struck = target.stats.struck + 1
        target.stats.taken  = target.stats.taken + dealt
        if dealt > 0 then
            target.struck = self.now
        end
        if target.hp == 0 then
            target.dead = true
            self.map:leave(target)
        end
    end
end

function World:remember(mob, attacker)
    if attacker == nil then
        return
    end
    local record          = mob.damaged[attacker] or { count = 0, at = 0 }
    record.count          = record.count + 1
    record.at             = self.now
    mob.damaged[attacker] = record
    if mob.locked == nil or self.now - (mob.lock or 0) > RULES.lock then
        mob.locked = attacker
        mob.lock   = self.now
    end
end

function World:kill(mob, killer)
    mob.dead = true
    self.map:leave(mob)
    self.mobs[mob.oid] = nil
    self.killed[mob.oid] = true
    local rezen = mob.rezen
    if rezen.due == nil then
        rezen.due = self.now + (rezen.spawn.rezen or 0) / (self.options.rezen_multiplier or 1)
    end
    rezen.alive = math.max(0, rezen.alive - 1)
    if killer ~= nil and killer.stats ~= nil then
        killer.stats.kills = killer.stats.kills + 1
        if self.share ~= nil then
            self.share(killer, mob.data.exp or 0)
        else
            self:award(killer, mob.data.exp or 0)
        end
    end
    mob.killer = killer
    if self.options.drops then
        self:scatter(mob)
    end
end

function World:scatter(mob)
    self.piles = self.piles or {}
    for _, drop in ipairs(mob.data.drops or {}) do
        if math.random() < drop.chance then
            self.pile_oid = (self.pile_oid or 0) + 1
            self.piles[self.pile_oid] = {
                oid = self.pile_oid,
                x = mob.x,
                y = mob.y,
                names = { drop.name },
                count = drop.count,
                capacity = drop.capacity,
            }
        end
    end
end

function World:award(character, exp)
    local multiplier = self.options.exp_multiplier or 1
    local value      = math.floor(exp * multiplier)
    local table      = growth[character.class] or {}
    local current    = table[character.level]
    if self.options.exp_limit ~= false and current ~= nil and table[character.level + 1] ~= nil then
        value = math.min(value,
            math.floor((table[character.level + 1].exp - current.exp) / 100.0 * RULES.limit * multiplier + 1))
    end
    character.exp       = character.exp + value
    character.stats.exp = character.stats.exp + value
    while table[character.level + 1] ~= nil and character.exp >= table[character.level + 1].exp do
        local now, after = table[character.level], table[character.level + 1]
        character.max_hp = character.max_hp + after.hp - now.hp
        character.max_mp = character.max_mp + after.mp - now.mp
        character.hp     = character.max_hp
        character.mp     = character.max_mp
        character.grown  = character.grown or { str = 0, int = 0, dex = 0 }
        for _, key in ipairs({ "str", "int", "dex" }) do
            character.grown[key] = character.grown[key] + after[key] - now[key]
        end
        character.level = character.level + 1
    end
end

local function distance(a, b)
    return math.abs(a.x - b.x) + math.abs(a.y - b.y)
end

function World:present(target)
    if target == nil or target.dead then
        return false
    end
    return target.data ~= nil or self.characters[target] == true
end

function World:adjacent(mob, target)
    local DIRECTION = self.enums.DIRECTION
    if self:present(target) == false or distance(mob, target) ~= 1 then
        return nil
    end
    if target.x < mob.x then
        return DIRECTION.LEFT
    elseif target.x > mob.x then
        return DIRECTION.RIGHT
    elseif target.y < mob.y then
        return DIRECTION.TOP
    end
    return DIRECTION.BOTTOM
end

function World:move(object, direction)
    local allowed, x, y = self.map:passable(object.x, object.y, direction, object)
    if allowed ~= true then
        return false
    end
    object.facing = direction
    self.map:shift(object, x, y)
    return true
end

function World:wander(mob)
    self:move(mob, self:random(0, 3))
end

function World:approach(mob, target)
    local DIRECTION = self.enums.DIRECTION
    local order     = self:random(0, 1) == 1 and { "x", "y" } or { "y", "x" }
    for _, axis in ipairs(order) do
        if axis == "x" then
            if mob.x > target.x and self:move(mob, DIRECTION.LEFT) then
                return true
            end
            if mob.x < target.x and self:move(mob, DIRECTION.RIGHT) then
                return true
            end
        else
            if mob.y > target.y and self:move(mob, DIRECTION.TOP) then
                return true
            end
            if mob.y < target.y and self:move(mob, DIRECTION.BOTTOM) then
                return true
            end
        end
    end
    return false
end

function World:flee(mob, target)
    local DIRECTION = self.enums.DIRECTION
    local count     = self:random(0, 1) == 1 and 2 or 1
    for _ = 1, count do
        local before = distance(mob, target)
        local moved  = false
        for _, want in ipairs({ 1, 0, -1 }) do
            if moved then
                break
            end
            for _, direction in ipairs({ DIRECTION.LEFT, DIRECTION.TOP, DIRECTION.RIGHT, DIRECTION.BOTTOM }) do
                local allowed, x, y = self.map:passable(mob.x, mob.y, direction, mob)
                if allowed == true and math.abs(x - target.x) + math.abs(y - target.y) - before == want and
                    self:move(mob, direction) then
                    moved = true
                    break
                end
            end
        end
    end
end

function World:visible(mob, target)
    return self:present(target) and self.map:sight(mob, target)
end

function World:forget(mob)
    for attacker, record in pairs(mob.damaged) do
        if attacker.dead or self.now - record.at > RULES.memory then
            mob.damaged[attacker] = nil
        end
    end
end

function World:sighted(mob)
    local best, most = nil, 0
    for attacker, record in pairs(mob.damaged) do
        if self:visible(mob, attacker) and record.count > most then
            best, most = attacker, record.count
        end
    end
    if best ~= nil then
        return best
    end
    return self:nearest(mob, false)
end

function World:ranged(mob)
    local best, most = nil, 0
    for attacker, record in pairs(mob.damaged) do
        if self:visible(mob, attacker) and self:adjacent(mob, attacker) ~= nil and record.count > most then
            best, most = attacker, record.count
        end
    end
    if best ~= nil then
        return best
    end
    return self:nearest(mob, true)
end

function World:lock(mob, target)
    mob.locked = target
    mob.lock   = self.now
end

function World:held(mob)
    return mob.locked ~= nil and self.now - (mob.lock or 0) <= RULES.lock
end

function World:strike(mob, direction)
    mob.facing = direction
    self.on_attack(mob.facade)
end

function World:chase(mob, target)
    if self:approach(mob, target) == false then
        self:wander(mob)
    end
end

local BEHAVIORS = {}

function BEHAVIORS.AGGRESSIVE(world, mob)
    local target = mob.locked
    if target == nil or target.dead or world:held(mob) == false then
        target = world:sighted(mob)
        if target ~= nil then
            world:lock(mob, target)
        end
    end
    if target == nil then
        world:wander(mob)
        return
    end
    local adjacent = world:adjacent(mob, target) ~= nil and target or world:ranged(mob)
    local facing   = world:adjacent(mob, adjacent)
    if facing ~= nil then
        if adjacent ~= target then
            world:lock(mob, adjacent)
        end
        world:strike(mob, facing)
    else
        world:chase(mob, target)
    end
end

function BEHAVIORS.RETALIATE(world, mob)
    local target = mob.locked
    if target == nil or target.dead then
        world:wander(mob)
        return
    end
    local facing = world:adjacent(mob, target)
    if facing ~= nil then
        world:strike(mob, facing)
    else
        world:chase(mob, target)
    end
end

function BEHAVIORS.RUN_AWAY(world, mob)
    local target = mob.locked
    if target == nil or target.dead or world:held(mob) == false then
        target = world:sighted(mob)
        if target ~= nil then
            world:lock(mob, target)
        end
    end
    if target == nil then
        world:wander(mob)
        return
    end
    if mob.hp / mob.max_hp <= RULES.flee then
        world:flee(mob, target)
        return
    end
    local facing = world:adjacent(mob, target)
    if facing ~= nil then
        world:strike(mob, facing)
    else
        world:chase(mob, target)
    end
end

function BEHAVIORS.STATIONARY(world, mob)
    local target = mob.locked
    if target == nil or target.dead or world:held(mob) == false then
        target = world:ranged(mob)
        if target ~= nil then
            world:lock(mob, target)
        end
    end
    local facing = world:adjacent(mob, target)
    if facing ~= nil then
        world:strike(mob, facing)
    end
end

function BEHAVIORS.NONE(world, mob)
    if mob.locked == nil or mob.locked.dead or world:held(mob) == false then
        mob.locked = nil
        world:wander(mob)
        return
    end
    world:flee(mob, mob.locked)
end

function BEHAVIORS.INERT()
end

function World:buff(object, model, seconds)
    local existing = object.buffs[model.label]
    if existing ~= nil then
        existing.expires = self.now + seconds * 1000
        return true
    end
    object.buffs[model.label] = { model = model, expires = self.now + seconds * 1000 }
    spells.hook(self, object.facade, model, "on_buff")
    return true
end

function World:unbuff(object, name)
    local buff = object.buffs[name]
    if buff == nil then
        return false
    end
    object.buffs[name] = nil
    spells.hook(self, object.facade, buff.model, "on_unbuff")
    return true
end

function World:expire(object)
    for name, buff in pairs(object.buffs) do
        if self.now >= buff.expires then
            self:unbuff(object, name)
        end
    end
end

function World:hook(object, path, name, ...)
    local id = object.data ~= nil and object.data.id or nil
    if id == nil or object:is(self.enums.OBJECT_TYPE.MOB) == false then
        return nil
    end
    self.hooks = self.hooks or {}
    if self.hooks[id] == nil then
        local file = io.open(string.format("../game/scripts/mob/%d.lua", id), "rb")
        local text = file ~= nil and file:read("a") or ""
        if file ~= nil then
            file:close()
        end
        self.hooks[id] = text:find(name, 1, true) ~= nil and self.runtime.load(string.format("mob/%d.lua", id)) or false
    end
    local script = self.hooks[id]
    if script == false or script[name] == nil then
        return nil
    end
    return script[name](object.facade, ...)
end

function World:act(mob)
    if mob.paralyzed or ((mob.cc or 0) & self.enums.CROWD_CONTROL.SIGHT) ~= 0 then
        return
    end
    if mob.locked ~= nil and self:present(mob.locked) == false then
        mob.locked = nil
    end
    self:forget(mob)
    local behavior = BEHAVIORS[mob.data.attack or "NONE"] or BEHAVIORS.NONE
    behavior(self, mob)
end

function World:recover(character)
    if character.dead then
        return
    end
    for kind, rule in pairs(RULES.recovery) do
        local current, maximum = character[kind], character["max_" .. kind]
        if current >= maximum then
            character.recovery[kind] = nil
        elseif character.recovery[kind] == nil then
            character.recovery[kind] = self.now + rule.every
        elseif self.now >= character.recovery[kind] then
            local amount = maximum * rule.rate * (kind == "hp" and (100 + character.regen) / 100 or 1)
            character[kind] = current + math.max(1, math.min(math.floor(amount), maximum - current))
            character.recovery[kind] = self.now + rule.every
        end
    end
end

function M.rest(character, duration)
    if character.dead then
        return
    end
    for kind, rule in pairs(RULES.recovery) do
        local ticks   = duration // rule.every
        local maximum = character["max_" .. kind]
        for _ = 1, ticks do
            local current = character[kind]
            if current >= maximum then
                break
            end
            local amount    = maximum * rule.rate * (kind == "hp" and (100 + character.regen) / 100 or 1)
            character[kind] = current + math.max(1, math.min(math.floor(amount), maximum - current))
        end
    end
end

function World:respawn()
    for _, rezen in ipairs(self.rezens) do
        if rezen.due ~= nil and self.now >= rezen.due then
            local missing = rezen.spawn.count - rezen.alive
            rezen.due     = nil
            if missing > 0 then
                self:populate(rezen, missing)
            end
        end
    end
end

function World:advance(duration)
    local target = self.now + duration
    while self.now < target do
        local next = (self.now // RULES.tick + 1) * RULES.tick
        self.now   = math.min(next, target)
        if self.now == next then
            self:respawn()
            local order = {}
            for _, mob in pairs(self.mobs) do
                table.insert(order, mob)
            end
            table.sort(order, function(left, right)
                return left.oid < right.oid
            end)
            for _, mob in ipairs(order) do
                self:expire(mob)
                if mob.dead == false and self.now >= mob.acted + (mob.data.speed or 1000) then
                    mob.acted = self.now
                    self:act(mob)
                end
            end
            for character in pairs(self.characters) do
                self:recover(character)
                self:expire(character)
            end
        end
    end
end

function World:swing(character)
    character.stats.swings = character.stats.swings + 1
    local hits             = character.stats.hits
    self.on_attack(character.facade)
    if character.stats.hits == hits then
        character.stats.misses = character.stats.misses + 1
    end
end

function World:find(oid)
    if self.mobs[oid] ~= nil then
        return self.mobs[oid]
    end
    for character in pairs(self.characters) do
        if character.oid == oid then
            return character
        end
    end
    return nil
end

return M
