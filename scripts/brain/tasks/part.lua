local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")

local M = {}

local function waited(snap)
    return math.max(0, snap.now - snap.record.memory.apart) / 1000
end

local function worth(snap, query)
    local plan    = party.plan(snap, query)
    local alone   = ground.choose(snap, query, ground.mode(snap))
    local group   = plan ~= nil and plan.rate or 0
    local solo    = alone ~= nil and alone.rate or 0
    local horizon = ground.horizon(snap, math.max(group, solo))
    if horizon == math.huge then
        return 0
    end
    return solo * horizon - group * math.max(0, horizon - waited(snap))
end

function M.wanted(snap, query)
    return snap.self.ghost == false and #(snap.group or {}) > 0 and snap.record.memory.apart ~= nil and
        worth(snap, query) > 0
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
