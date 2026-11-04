local M = {}

local TABLE = "../game/json/spell.json"

local index = nil

function M.index()
    if index ~= nil then
        return index
    end
    index      = {}
    local file = assert(io.open(TABLE, "rb"))
    local text = file:read("a")
    file:close()
    for id, name, kind in text:gmatch('"id":%s*(%d+),%s*"name":%s*"(.-)",%s*"type":%s*"(%u+)"') do
        if index[name] == nil then
            index[name] = { id = tonumber(id), kind = kind }
        end
    end
    return index
end

function M.model(world, name)
    local entry = M.index()[name]
    if entry == nil then
        return nil
    end
    local id   = entry.id
    local kind = world.enums.SPELL_TYPE[entry.kind]
    return {
        key = id,
        label = name,
        kind = kind,
        id = function()
            return id
        end,
        name = function()
            return name
        end,
        type = function()
            return kind
        end,
        message = function()
            return ""
        end,
    }
end

local scripts = {}

function M.script(world, id)
    if scripts[id] == nil then
        local ok, loaded = pcall(world.runtime.load, string.format("spell/%d.lua", id))
        scripts[id] = ok and (loaded or false) or false
        if ok == false then
            error(loaded)
        end
    end
    return scripts[id] or nil
end

function M.hook(world, object, model, name)
    local script = M.script(world, model.key)
    local hook   = script ~= nil and script[name] or nil
    if hook ~= nil then
        hook(object, model)
    end
end

function M.cast(world, me, name, target)
    local model = M.model(world, name)
    if model == nil then
        return false, "NO_SPELL"
    end
    local script = M.script(world, model.key)
    if script == nil or script.on_cast == nil then
        return false, "NO_SCRIPT"
    end
    local SPELL_TYPE = world.enums.SPELL_TYPE
    if model.kind == SPELL_TYPE.TARGET then
        if target == nil or target.inner.dead or world.map:sight(me.inner, target.inner) == false then
            return true
        end
        script.on_cast(me, target, model)
    elseif model.kind == SPELL_TYPE.INPUT then
        script.on_cast(me, model, "")
    else
        script.on_cast(me, model)
    end
    return true
end

return M
