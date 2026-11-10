local M = {}

local State = {}
State.__index = State

local Life      = {}
Life.__index    = Life
Life.__userdata = true

local function model(world, data)
    return {
        size = function()
            return world.enums.MOB_SIZE[data.size or "SMALL"]
        end,
        name = function()
            return data.name
        end,
        id = function()
            return data.id
        end,
        type = function()
            return data.weapon
        end,
        sound = function()
            return data.sound or 0
        end,
        option = function()
            return world.map.option
        end,
        siege_castle = function()
            return nil
        end,
    }
end

local function face(object)
    return object ~= nil and object.facade or nil
end

local function surface(world)
    local map = world.map
    return {
        width = function()
            return map.width
        end,
        height = function()
            return map.height
        end,
        at = function(_, x, y, kind)
            return face(map:at(x, y, kind)[1])
        end,
        movable = function(_, who, first, second)
            if second == nil then
                return map:passable(who.inner.x, who.inner.y, first, who.inner) == true
            end
            return map:vacant(first, second, who.inner)
        end,
        objects = function()
            local found = {}
            for object in pairs(map.objects) do
                table.insert(found, object.facade)
            end
            return found
        end,
        model = function()
            return model(world, {})
        end,
    }
end

function M.life(world, fields)
    local object     = setmetatable(fields, State)
    object.world     = world
    object.buffs     = object.buffs or {}
    object.dam_value = object.dam_value or 0
    object.hit_value = object.hit_value or 0
    object.rate      = object.rate or 1000
    object.derate    = object.derate or 1000
    object.skill     = object.skill or 1000
    object.dead      = false
    object.facade    = setmetatable({ inner = object }, Life)
    return object
end

function State:alive()
    return self.dead == false
end

function State:is(kind)
    return (self.type & kind) == kind
end

function Life:is(kind)
    return self.inner:is(kind)
end

function Life:map()
    return surface(self.inner.world)
end

function Life:name()
    return self.inner.label
end

function Life:option()
    return false
end

function Life:action()
end

function Life:sound()
end

function Life:effect()
end

function Life:message()
end

function Life:chat()
end

function Life:update()
end

function Life:owner()
    return nil
end

function Life:clan()
    return nil
end

function Life:weapon()
    local inner = self.inner
    if inner.arm == nil then
        return nil
    end
    return {
        model = function()
            return model(inner.world, inner.arm)
        end,
    }
end

function Life:model()
    return model(self.inner.world, self.inner.data or {})
end

function Life:position(x, y)
    local inner = self.inner
    if x ~= nil and y ~= nil then
        inner.world.map:shift(inner, x, y)
    end
    return inner.x, inner.y
end

function Life:direction(value)
    if value ~= nil then
        self.inner.facing = value
    end
    return self.inner.facing
end

function Life:front(kind)
    local inner = self.inner
    local x, y  = inner.world.map:step(inner.x, inner.y, inner.facing)
    return face(inner.world.map:at(x, y, kind)[1])
end

function Life:front_position()
    local inner = self.inner
    return inner.world.map:step(inner.x, inner.y, inner.facing)
end

function Life:nears(kind, points)
    local inner = self.inner
    local found = {}
    if points == nil then
        points = {}
        for _, offset in ipairs({ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }) do
            table.insert(points, { inner.x + offset[1], inner.y + offset[2] })
        end
    end
    for _, point in ipairs(points) do
        for _, object in ipairs(inner.world.map:at(point[1], point[2], kind)) do
            table.insert(found, object.facade)
        end
    end
    return found
end

function Life:target(...)
    if select("#", ...) > 0 then
        local target = ...
        self.inner.world:provoke(self.inner, target ~= nil and target.inner or nil)
        return
    end
    return face(self.inner.locked)
end

function Life:isbuff(...)
    for _, name in ipairs({ ... }) do
        if self.inner.buffs[name] ~= nil then
            return true
        end
    end
    return false
end

function Life:buff(spell, seconds)
    return self.inner.world:buff(self.inner, spell, seconds)
end

function Life:unbuff(name)
    self.inner.world:unbuff(self.inner, name)
end

function Life:hit()
    return self.inner.hit_value
end

function Life:dam()
    return self.inner.dam_value
end

function Life:damage_rate(value)
    if value ~= nil then
        self.inner.rate = value
    end
    return self.inner.rate
end

function Life:damage_derate(value)
    if value ~= nil then
        self.inner.derate = value
    end
    return self.inner.derate
end

function Life:skill_damage_rate(value)
    if value ~= nil then
        self.inner.skill = value
    end
    return self.inner.skill
end

function Life:hp(value)
    local inner = self.inner
    if value ~= nil then
        inner.hp = math.max(0, math.min(inner.max_hp, value))
    end
    return inner.hp
end

function Life:maxhp()
    return self.inner.max_hp
end

function Life:mp(value)
    local inner = self.inner
    if value ~= nil then
        inner.mp = math.max(0, math.min(inner.max_mp or 0, value))
    end
    return inner.mp or 0
end

function Life:maxmp()
    return self.inner.max_mp or 0
end

function Life:mp_down(value)
    self.inner.mp = math.max(0, (self.inner.mp or 0) - value)
end

function Life:mp_up(value)
    self.inner.mp = math.min(self.inner.max_mp or 0, (self.inner.mp or 0) + value)
end

function Life:heal(value)
    local inner = self.inner
    inner.hp    = math.min(inner.max_hp, inner.hp + value)
end

function Life:assert(...)
    local inner = self.inner
    local STATE = inner.world.enums.STATE
    for _, state in ipairs({ ... }) do
        if state == STATE.GHOST and inner.dead then
            return "귀신은 할 수 없습니다."
        end
    end
    return nil
end

function Life:state(value)
    local inner = self.inner
    if value ~= nil then
        inner.state = value
        return value
    end
    if inner.dead then
        return inner.world.enums.STATE.GHOST
    end
    return inner.state or inner.world.enums.STATE.NORMAL
end

function Life:level()
    return self.inner.level or 0
end

function Life:resist()
    return 0
end

local function buffed(object, field, stat, value)
    local inner = object.inner
    inner.buffed = inner.buffed or {}
    local before = inner.buffed[field] or 0
    if value ~= nil then
        inner.buffed[field] = value
        if stat ~= nil then
            inner[stat] = (inner[stat] or 0) + value - before
        end
    end
    return inner.buffed[field] or 0
end

function Life:buff_phydef(value)
    return buffed(self, "phydef", "phydef", value)
end

function Life:buff_hit(value)
    return buffed(self, "hit", "hit_value", value)
end

function Life:buff_dam(value)
    return buffed(self, "dam", "dam_value", value)
end

function Life:buff_str(value)
    return buffed(self, "str", nil, value)
end

function Life:buff_int(value)
    return buffed(self, "int", nil, value)
end

function Life:buff_dex(value)
    return buffed(self, "dex", nil, value)
end

local function stat(object, field)
    local inner = object.inner
    return ((inner.stats or {})[field] or 0) + ((inner.buffed or {})[field] or 0)
end

function Life:str()
    return stat(self, "str")
end

function Life:int()
    return stat(self, "int")
end

function Life:dex()
    return stat(self, "dex")
end

function Life:class()
    local inner = self.inner
    return inner.world.enums.CLASS[inner.class or "NONE"] or 0
end

function Life:promotion()
    return self.inner.promotion or 0
end

function Life:add_cc(value)
    local inner = self.inner
    if value ~= nil then
        inner.cc = (inner.cc or 0) | value
    end
    return inner.cc or 0
end

function Life:remove_cc(value)
    local inner = self.inner
    inner.cc    = (inner.cc or 0) & ~value
    return inner.cc
end

function Life:delirious(value)
    if value ~= nil then
        self.inner.delirious = value
    end
    return self.inner.delirious == true
end

function Life:damage(value)
    self.inner.world:damage(nil, self.inner, value, { fixed = true })
end

function Life:paralysis(value)
    if value ~= nil then
        self.inner.paralyzed = value
    end
    return self.inner.paralyzed == true
end

function Life:script(path, name, ...)
    return self.inner.world:hook(self.inner, path, name, ...)
end

function Life:attack()
    return self.inner.world.on_attack(self)
end

function Life:normal_attack_damage(size)
    return self.inner.world:roll(self.inner, size)
end

function Life:damage_to(you, value, options)
    local world = self.inner.world
    if getmetatable(you) == Life then
        world:damage(self.inner, you.inner, value, options or {})
        return
    end
    for _, entry in ipairs(you) do
        world:damage(self.inner, entry[1].inner, entry[2], value or {})
    end
end

return M
