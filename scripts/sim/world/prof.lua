local M = {}

local stats   = {}
local stack   = {}
local depth   = {}
local started = nil

local function wrap(name, fn)
    return function(...)
        local entry = stats[name]
        if entry == nil then
            entry = { calls = 0, total = 0, own = 0 }
            stats[name] = entry
        end
        local frame = { begin = os.clock(), child = 0 }
        stack[#stack + 1] = frame
        depth[name] = (depth[name] or 0) + 1
        local result = table.pack(fn(...))
        local spent = os.clock() - frame.begin
        stack[#stack] = nil
        depth[name] = depth[name] - 1
        entry.calls = entry.calls + 1
        entry.own = entry.own + spent - frame.child
        if depth[name] == 0 then
            entry.total = entry.total + spent
        end
        local parent = stack[#stack]
        if parent ~= nil then
            parent.child = parent.child + spent
        end
        return table.unpack(result, 1, result.n)
    end
end

local function patch(label, target)
    for key, value in pairs(target) do
        if type(value) == "function" then
            target[key] = wrap(label .. "." .. key, value)
        end
    end
end

function M.install()
    started = os.clock()
    require("sim.world.driver")
    require("brain.decide")
    for name, loaded in pairs(package.loaded) do
        if type(loaded) == "table" and (name:find("^brain%.policies%.") or name:find("^brain%.tasks%.") or
                name == "brain.decide" or name == "brain.snapshot") then
            patch((name:gsub("^brain%.", "")), loaded)
        end
    end
    local facts = require("sim.facts")
    local bind  = facts.bind
    facts.bind  = function(...)
        local bound = bind(...)
        patch("query", bound)
        return bound
    end
end

function M.report()
    local names = {}
    for name in pairs(stats) do
        table.insert(names, name)
    end
    table.sort(names, function(left, right)
        return stats[left].own > stats[right].own
    end)
    log("debug", string.format("[sim:prof] wall=%.1fs", os.clock() - started))
    for index = 1, math.min(#names, 45) do
        local entry = stats[names[index]]
        log("debug", string.format("[sim:prof] %s calls=%d own=%.2fs total=%.2fs", names[index], entry.calls, entry.own,
            entry.total))
    end
end

return M
