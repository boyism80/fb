local M = {}

local SCRIPTS = "../game/scripts/"
local HEADER  = "../../include/fb/model/model.h"
local STUBS   = { ["lib.npc"] = true, ["lib.command"] = true }
local BASICS  = { "assert", "error", "ipairs", "next", "pairs", "pcall", "rawequal", "rawget", "rawlen", "rawset",
    "select", "setmetatable", "getmetatable", "tonumber", "tostring", "type", "xpcall", "string", "table", "math",
    "utf8" }

local enums = nil

local function evaluate(expression, scope)
    local text = expression:gsub("%f[%w_][%a_][%w_]*", function(name)
        if scope[name] == nil then
            error("unknown enum reference " .. name)
        end
        return tostring(scope[name])
    end)
    return assert(load("return " .. text))()
end

function M.enums()
    if enums ~= nil then
        return enums
    end
    local file = assert(io.open(HEADER, "rb"))
    local text = file:read("a")
    file:close()
    local parsed = {}
    for name, body in text:gmatch("enum class ([%w_]+)[^{;]-{(.-)}") do
        local scope = {}
        for entry in body:gmatch("[^,]+") do
            local key, expression = entry:match("^%s*([%w_]+)%s*=%s*(.-)%s*$")
            if key ~= nil then
                scope[key] = evaluate(expression, scope)
            end
        end
        parsed[name] = scope
    end
    enums = parsed
    return enums
end

local function stub(name)
    return setmetatable({}, {
        __index = function(_, key)
            error(string.format("server module %s.%s is not available offline", name, tostring(key)), 2)
        end,
    })
end

function M.new(globals)
    local env    = {}
    local loaded = {}
    for _, name in ipairs(BASICS) do
        env[name] = _G[name]
    end
    env.unpack    = table.unpack
    env.type      = function(value)
        local meta = getmetatable(value)
        if type(value) == "table" and type(meta) == "table" and rawget(meta, "__userdata") then
            return "userdata"
        end
        return type(value)
    end
    env.name_with = function(name)
        return name
    end
    for name, values in pairs(M.enums()) do
        env[name] = values
    end
    env.debug = function()
        return false
    end
    for name, value in pairs(globals or {}) do
        env[name] = value
    end
    env.require = function(name)
        if loaded[name] ~= nil then
            return loaded[name]
        end
        if STUBS[name] then
            loaded[name] = stub(name)
            return loaded[name]
        end
        local path   = SCRIPTS .. name:gsub("%.", "/") .. ".lua"
        local chunk  = assert(loadfile(path, "t", env))
        loaded[name] = chunk(name) or true
        return loaded[name]
    end
    setmetatable(env, {
        __index = function(_, key)
            error(string.format("server global %s is not available offline", tostring(key)), 2)
        end,
    })
    local runtime = { env = env }
    function runtime.load(path)
        local chunk = assert(loadfile(SCRIPTS .. path, "t", env))
        return chunk()
    end
    return runtime
end

return M
