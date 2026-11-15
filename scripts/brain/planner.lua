local M = {}

-- What crosses to the planning thread is copied as data (fb::lua::value): a metatable is not followed, so what a table
-- inherits through __index (an evaluated floor shares its whole assessment that way) is copied as its own, functions
-- are left out, and a table met again inside itself (a chosen plan lists itself among its options) is cut there.
local function plain(source, inside)
    inside[source] = true
    local found = {}
    local layer = source
    while layer ~= nil do
        for key, entry in pairs(layer) do
            local kind = type(entry)
            if found[key] == nil and (type(key) == "string" or type(key) == "number") then
                if kind == "table" then
                    if inside[entry] == nil then
                        found[key] = plain(entry, inside)
                    end
                elseif kind ~= "function" and kind ~= "userdata" and kind ~= "thread" then
                    found[key] = entry
                end
            end
        end
        local meta = getmetatable(layer)
        layer      = meta ~= nil and type(meta.__index) == "table" and meta.__index or nil
    end
    inside[source] = nil
    return found
end

function M.plain(source)
    return plain(source, {})
end

local prints = setmetatable({}, { __mode = "k" })

function M.fingerprint(snap, query)
    if prints[snap] == nil then
        local party  = require("brain.policies.party")
        local score  = require("brain.policies.score")
        prints[snap] = party.key(snap, query) .. "#" .. score.signature(snap)
    end
    return prints[snap]
end

-- The planning thread works on a copy of the snapshot with no bot behind it: the walks, reaches and routes are asked of
-- the atlas from the place the copy stands (sim.facts), and what other bots take of a ground is read from the ledger.
function M.bind(brain, snap)
    local query = require("sim.facts").bind(brain, snap, {})
    query.demand = function(map, crew)
        local except = { snap.self.name }
        for _, name in ipairs(crew or {}) do
            table.insert(except, name)
        end
        return brain:demand(nil, map, except)
    end
    return query
end

function M.compute(snap, query)
    local ground = require("brain.policies.ground")
    local party  = require("brain.policies.party")
    local value  = require("brain.policies.value")
    return {
        plan     = party.compute(snap, query),
        mana     = value.mana(snap, query),
        exchange = value.exchange(snap, query),
        chosen   = ground.choose(snap, query),
    }
end

function M.run(brain, snap)
    return plain(M.compute(snap, M.bind(brain, snap)), {})
end

local received = {}
local asked    = {}
local current  = setmetatable({}, { __mode = "k" })
local stale    = setmetatable({}, { __mode = "k" })
local noted    = {}

local function own(key)
    return key:match("^[^|]*|(.*)$") or key
end

-- A decision reads the newest plan the planning thread made for its bot and never works one out itself: one worked
-- out in the decision held every bot of the thread, and a mage stood four seconds under skeletons until it died
-- (online 215552). A plan made in an earlier period is used until the one asked for comes, as only time moved since.
-- One made for another state of the bot (before it committed to a ground, before its crew was known) is read too,
-- and the decision is not held for the plan of its state: worked out anew the plan takes one to three seconds at 99,
-- and waiting for it held a poet at 137 hp and the poet that would heal it 1.4-3.3s at a time until a mob came up
-- and killed the first (online 20261115-132144, 13:25:49-56). What it would undo is left undone instead (stale): a
-- leader committed to the plan made before its crew was known, and another took a plan made before it committed and
-- went back and forth between 흑령굴 and 흑해골굴 within a second, and two groups ended on one ground (online 004028,
-- 00:41:15-18), so a decision on a stale plan does not commit, nor start what goes where the plan says (decide). With
-- no plan yet the decision still waits. A copy of the snapshot that plans the same (its mana refilled, the same build)
-- reads what the decision's own snapshot reads; one that plans otherwise is worked out where it is asked.
local function read(snap, query)
    local key = M.fingerprint(snap, query)
    if query.snap ~= snap then
        if query.snap ~= nil and key == M.fingerprint(query.snap, query) then
            return read(query.snap, query)
        end
        return nil, nil
    end
    local known = current[snap]
    if known ~= nil then
        return known.value, known.stale
    end
    local name = snap.self.name
    local function fetch(kept)
        local found = query.planned(kept ~= nil and kept.version or nil)
        if found ~= nil and found.value ~= nil then
            kept           = { version = found.version, key = found.key, value = found.value }
            received[name] = kept
        end
        return kept
    end
    local kept = fetch(received[name])
    if (kept == nil or kept.key ~= key) and asked[name] ~= key then
        asked[name] = key
        query.plan(key, plain(snap, {}))
    end
    if kept == nil then
        query.await_plan(key)
        kept = fetch(kept)
    end
    local result = kept ~= nil and { value = kept.value, stale = own(kept.key) ~= own(key) and key or nil } or nil
    if result ~= nil then
        current[snap] = result
    end
    if result ~= nil and result.stale ~= nil and noted[name] ~= key then
        noted[name] = key
        local before, after, moved = {}, {}, {}
        for part in (own(kept.key) .. "|"):gmatch("([^|]*)|") do
            table.insert(before, part)
        end
        for part in (own(key) .. "|"):gmatch("([^|]*)|") do
            table.insert(after, part)
        end
        for index = 1, math.max(#before, #after) do
            if before[index] ~= after[index] then
                table.insert(moved, string.format("%d:%s>%s", index, tostring(before[index]), tostring(after[index])))
            end
        end
        log("debug", string.format("[brain:stale] bot=%s parts=%d/%d moved=%s", name, #before, #after,
            table.concat(moved, " ", 1, math.min(#moved, 8))))
    end
    return result ~= nil and result.value or nil, result ~= nil and result.stale or nil
end

-- What decides where the bot goes (its plan, its ground) reads the plan through current, which marks the decision when
-- the plan is of another state; what only prices time and mana reads it through latest.
function M.current(snap, query)
    if query.planned == nil then
        return nil
    end
    local value, key = read(snap, query)
    if key ~= nil then
        stale[query.snap or snap] = key
    end
    return value
end

function M.latest(snap, query)
    if query.planned == nil then
        return nil
    end
    return (read(snap, query))
end

function M.stale(snap)
    return stale[snap]
end

return M
