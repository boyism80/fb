local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")
local score  = require("brain.policies.score")
local field  = require("brain.tasks.field")

local M = {}

local function waited(snap)
    return math.max(0, snap.now - (snap.record.memory.apart or snap.now)) / 1000
end

local function worth(snap, query)
    if party.known(snap) == false then
        return 0
    end
    local base    = score.recovered(snap, query)
    local plan    = party.plan(base, query)
    local alone   = ground.choose(base, query, ground.mode(base))
    local group   = plan ~= nil and plan.rate or 0
    local solo    = alone ~= nil and alone.rate or 0
    local horizon = ground.horizon(snap, math.max(group, solo))
    if horizon == math.huge then
        return 0
    end
    return solo * horizon - group * math.max(0, horizon - waited(snap))
end

local function stricken(snap)
    if snap.self.ghost then
        return true
    end
    for _, member in ipairs(party.members(snap)) do
        if member.self.ghost then
            return true
        end
    end
    return false
end

local function finished(snap, query)
    local plan = party.plan(snap, query)
    if plan == nil then
        return true
    end
    local swept = snap.record.memory.swept
    if swept == nil or swept.name ~= plan.name then
        return false
    end
    for _, floor in ipairs(plan.floors) do
        if swept.floors[tostring(floor.map)] == nil then
            return false
        end
    end
    local last = plan.floors[#plan.floors]
    return last.map == snap.world.map and field.target(snap, query, last.targets) == nil
end

function M.wanted(snap, query)
    return #(snap.group or {}) > 0 and (stricken(snap) or finished(snap, query)) and worth(snap, query) > 0
end

function M.value(snap, query)
    return worth(snap, query)
end

function M.propose(snap)
    local names = {}
    for _, profile in ipairs(snap.party or {}) do
        table.insert(names, profile.self.name)
    end
    return { kind = ACTION.LEAVE, names = names, waited = waited(snap) }
end

return M
