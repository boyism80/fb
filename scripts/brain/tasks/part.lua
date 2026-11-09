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
    local group   = plan ~= nil and (plan.peak or plan.rate) or 0
    local solo    = alone ~= nil and (alone.peak or alone.rate) or 0
    local horizon = ground.horizon(snap, math.max(group, solo))
    if horizon == math.huge then
        return 0
    end
    return solo * horizon - group * math.max(0, horizon - waited(snap))
end

function M.stricken(snap)
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

function M.finished(snap, query)
    return party.plan(snap, query) == nil
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
