local data      = require("brain.data.combat")
local decide    = require("brain.decide")
local facts     = require("sim.facts")
local serialize = require("sim.serialize")
local trace     = require("sim.trace")
local json      = require("sim.world.json")

local M = {}

local function path_of(snap)
    local rows = decide.offers[snap.self.name or ""] or {}
    return rows[1] ~= nil and rows[1].path or nil
end

local function inputs(actor, snap, bound)
    local allies = {}
    for _, other in ipairs(actor.session.actors) do
        if other ~= actor then
            allies[other.base.self.name] = bound.ally(other.base.self.name)
        end
    end
    local posted = {}
    for _, profile in ipairs(snap.party or {}) do
        posted[profile] = true
    end
    local roster = {}
    for _, entry in ipairs(actor.session:roster(actor)) do
        table.insert(roster, posted[entry.profile] and { name = entry.name } or entry)
    end
    return { occupants = actor.session.occupants, roster = roster, allies = allies }
end

local function roster_of(moment)
    local named = {}
    for _, profile in ipairs(moment.snap.party or {}) do
        named[profile.self.name or ""] = profile
    end
    local roster = {}
    for _, entry in ipairs((moment.world or {}).roster or {}) do
        table.insert(roster, { name = entry.name, profile = entry.profile or named[entry.name] })
    end
    return roster
end

function M.keep(actor, snap, bound, decision)
    local options = actor.options
    local length  = options.recent or 0
    local every   = options.corpus ~= nil and options.sample or 0
    local memory  = actor.moments or { count = 0, list = {} }
    actor.moments = memory
    memory.count  = memory.count + 1
    local sampled = every > 0 and memory.count % every == 0
    if length <= 0 and sampled == false then
        return
    end
    local moment = serialize.frozen({
        scene = options.scene or "solo",
        seed  = actor.session.seed,
        bot   = actor.base.self.name,
        t     = actor.clock,
        order = memory.count,
        snap  = snap,
        world = inputs(actor, snap, bound),
        chose = trace.described(decision),
        path  = path_of(snap),
    })
    if length > 0 then
        memory.list[memory.count % length + 1] = moment
    end
    if sampled then
        local name = string.gsub(tostring(moment.bot), "[^%w_]", "_")
        local file = io.open(string.format("%s/%s.%s.moments", options.corpus, tostring(options.tag), name), "a")
        if file ~= nil then
            file:write(serialize.text(moment), "\n")
            file:close()
        end
    end
end

function M.write(path, list)
    if #list == 0 then
        return false
    end
    table.sort(list, function(left, right)
        if left.t ~= right.t then
            return left.t < right.t
        elseif left.bot ~= right.bot then
            return tostring(left.bot) < tostring(right.bot)
        end
        return left.order < right.order
    end)
    local file = io.open(path, "w")
    if file == nil then
        return false
    end
    for _, moment in ipairs(list) do
        file:write(serialize.text(moment), "\n")
    end
    file:close()
    return true
end

function M.spill(actor, path)
    local list = {}
    for _, moment in pairs(actor.moments ~= nil and actor.moments.list or {}) do
        table.insert(list, moment)
    end
    return M.write(path, list)
end

function M.lately(actor, count)
    local list = {}
    for _, other in ipairs(actor.session.actors) do
        local memory = other.moments
        local length = other.options ~= nil and other.options.recent or 0
        for back = 0, memory ~= nil and length > 0 and math.min(count, memory.count) - 1 or -1 do
            table.insert(list, memory.list[(memory.count - back) % length + 1])
        end
    end
    return list
end

function M.redo(mind, moment, traced)
    local snap  = moment.snap
    local world = moment.world or {}
    local bound = facts.bind(mind, snap, {
        occupants = world.occupants,
        roster = roster_of(moment),
        ally = function(name)
            return (world.allies or {})[name]
        end,
    })
    if traced then
        return trace.nodes(require("brain.trees.main"), function(seen, bound_)
            return decide.choose(seen, bound_, nil, nil)
        end, snap, bound)
    end
    return nil, decide.choose(snap, bound, nil, nil)
end

local function read(line)
    local chunk = load(line)
    if chunk == nil then
        return nil
    end
    local ok, value = pcall(chunk)
    if ok == false or type(value) ~= "table" then
        return nil
    end
    return value.snap ~= nil and value or { snap = value }
end

local function result_of(mind, moment, traced)
    local result              = {
        scene    = moment.scene,
        seed     = moment.seed,
        bot      = moment.snap.self.name,
        t        = moment.t or moment.snap.now,
        was      = moment.chose,
        was_path = moment.path,
    }
    local started             = os.clock()
    local ok, nodes, decision = pcall(M.redo, mind, moment, traced)
    result.ms                 = (os.clock() - started) * 1000
    if ok then
        result.now   = trace.described(decision)
        result.path  = path_of(moment.snap)
        result.nodes = nodes
    else
        result.error = tostring(nodes)
    end
    return result
end

function M.replay(options)
    local mind   = brain.new()
    local out    = assert(io.open(options.out, "w"))
    local stride = options.stride or 1
    local index  = 0
    local done   = 0
    local traced = data.trace
    data.trace   = true
    if options.patch ~= nil then
        dofile(options.patch)
    end
    for name in io.lines(options.list) do
        local row = 0
        for line in io.lines(name) do
            row        = row + 1
            index      = index + 1
            local mine = index % stride == 0 and (index // stride) % options.shards == options.shard
            if mine and (options.row == nil or options.row == row) then
                local moment = read(line)
                local result = moment ~= nil and result_of(mind, moment, options.row ~= nil) or
                    { error = "unreadable moment" }
                result.file  = name
                result.row   = row
                out:write(json.encode(result), "\n")
                done = done + 1
            end
        end
    end
    out:close()
    data.trace = traced
    log("debug", string.format("[sim:moments] shard=%d/%d replayed=%d of %d", options.shard, options.shards, done, index))
    return true
end

return M
