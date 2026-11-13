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
        local frame = { begin = os.clock(), child = 0, name = name }
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

local wrapped = setmetatable({}, { __mode = "k" })
local visited = setmetatable({}, { __mode = "k" })

local function inner(label, fn)
    if visited[fn] then
        return
    end
    visited[fn] = true
    for index = 1, math.huge do
        local name, value = debug.getupvalue(fn, index)
        if name == nil then
            break
        end
        if type(value) == "function" and wrapped[value] == nil then
            local info = debug.getinfo(value, "S")
            if info.what == "Lua" and info.source:find("brain[/\\]") ~= nil then
                local scoped     = label .. ":" .. name
                local wrapper    = wrap(scoped, value)
                wrapped[wrapper] = true
                debug.setupvalue(fn, index, wrapper)
                inner(label, value)
            end
        end
    end
end

local function patch(label, target, shallow)
    for key, value in pairs(target) do
        if type(value) == "function" then
            if shallow ~= true then
                inner(label, value)
            end
            target[key] = wrap(label .. "." .. key, value)
            wrapped[target[key]] = true
        end
    end
end

local sites    = {}
local choosers = {}
local ranks    = 0

local function fields(key)
    local found = {}
    for part in (tostring(key) .. "|"):gmatch("(.-)|") do
        found[#found + 1] = part
    end
    return found
end

local function novel(entry, owner, base, key)
    local seen = entry.seen[owner]
    if seen == nil then
        seen = { bases = {}, fields = {} }
        entry.seen[owner] = seen
        entry.owners = entry.owners + 1
    end
    local fresh = false
    if seen.bases[base] == nil then
        seen.bases[base] = true
        entry.bases = entry.bases + 1
        fresh = true
    end
    for index, part in ipairs(fields(key)) do
        local known = seen.fields[index]
        if known == nil then
            known = {}
            seen.fields[index] = known
        end
        if known[part] == nil then
            known[part] = true
            entry.fields[index] = (entry.fields[index] or 0) + 1
            fresh = true
        end
    end
    if fresh == false then
        entry.combos = entry.combos + 1
    end
end

local function watch_memo()
    local memo = require("brain.policies.memo")
    local get  = memo.get
    memo.get   = function(store, owner, base, key, compute)
        local info  = debug.getinfo(compute, "S")
        local name  = (info.short_src:match("[^/\\]+$") or "?") .. ":" .. info.linedefined
        local entry = sites[name]
        if entry == nil then
            entry = { calls = 0, misses = 0, time = 0, owners = 0, bases = 0, combos = 0, fields = {}, seen = {} }
            sites[name] = entry
        end
        entry.calls = entry.calls + 1
        return get(store, owner, base, key, function()
            entry.misses = entry.misses + 1
            novel(entry, owner or "", base, key)
            local begin = os.clock()
            local value = compute()
            entry.time  = entry.time + os.clock() - begin
            return value
        end)
    end
end

local function watch_choose()
    local ground  = require("brain.policies.ground")
    local ranked  = ground.ranked
    ground.ranked = function(...)
        ranks = ranks + 1
        return ranked(...)
    end
    local choose  = ground.choose
    ground.choose = function(snap, query, mode, crew, guards)
        local top   = stack[#stack]
        local name  = table.concat({ top ~= nil and top.name or "root", "mode=" .. tostring(mode),
            snap.hypothesis ~= nil and "hypothesis" or "plain", crew ~= nil and "crew" or "alone",
            guards ~= nil and "guards" or "open" }, ",")
        local entry = choosers[name]
        if entry == nil then
            entry = { calls = 0, misses = 0, time = 0 }
            choosers[name] = entry
        end
        local before = ranks
        local begin  = os.clock()
        local found  = choose(snap, query, mode, crew, guards)
        entry.calls  = entry.calls + 1
        entry.time   = entry.time + os.clock() - begin
        if ranks > before then
            entry.misses = entry.misses + 1
        end
        return found
    end
end

local callers = {}

local function watch_callers(module, name)
    local loaded = require("brain.policies." .. module)
    local inner  = loaded[name]
    local label  = module .. "." .. name
    loaded[name] = function(...)
        local top   = stack[#stack]
        local key   = label .. " " .. (top ~= nil and top.name or "root")
        local entry = callers[key]
        if entry == nil then
            entry = { calls = 0, time = 0 }
            callers[key] = entry
        end
        local begin = os.clock()
        local found = table.pack(inner(...))
        entry.calls = entry.calls + 1
        entry.time  = entry.time + os.clock() - begin
        return table.unpack(found, 1, found.n)
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
        patch("query", bound, true)
        for _, name in ipairs({ "path", "reach", "route", "steps", "leaps" }) do
            local inner = bound[name]
            if inner ~= nil then
                bound[name] = function(...)
                    local top   = stack[#stack]
                    local key   = "query." .. name .. " " .. (top ~= nil and top.name or "root")
                    local entry = callers[key]
                    if entry == nil then
                        entry = { calls = 0, time = 0 }
                        callers[key] = entry
                    end
                    local begin = os.clock()
                    local found = table.pack(inner(...))
                    entry.calls = entry.calls + 1
                    entry.time  = entry.time + os.clock() - begin
                    return table.unpack(found, 1, found.n)
                end
            end
        end
        return bound
    end
    watch_memo()
    watch_choose()
    for _, target in ipairs({ { "danger", "winnable" }, { "escort", "context" }, { "survival", "fight" }, { "combat", "outcome" },
        { "recovery", "rates" }, { "recovery", "purchasable" }, { "value", "baseline" }, { "battle", "choose" } }) do
        watch_callers(target[1], target[2])
    end
end

function M.report()
    local found = { wall = os.clock() - started, functions = {}, sites = {}, callers = {}, choosers = {} }
    for name, entry in pairs(stats) do
        found.functions[name] = { calls = entry.calls, own = entry.own, total = entry.total }
    end
    for name, entry in pairs(sites) do
        local fields = {}
        for index, count in pairs(entry.fields) do
            fields["f" .. index] = count
        end
        found.sites[name] = {
            calls = entry.calls,
            misses = entry.misses,
            time = entry.time,
            owners = entry.owners,
            bases = entry.bases,
            combos = entry.combos,
            fields = fields,
        }
    end
    for name, entry in pairs(callers) do
        found.callers[name] = { calls = entry.calls, time = entry.time }
    end
    for name, entry in pairs(choosers) do
        found.choosers[name] = { calls = entry.calls, misses = entry.misses, time = entry.time }
    end
    return found
end

return M
