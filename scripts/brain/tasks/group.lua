local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")

local M = {}

function M.wanted(snap, query)
    return party.offer(snap, query) ~= nil
end

function M.value(snap, query)
    local offer   = party.offer(snap, query)
    local horizon = ground.horizon(snap, offer.plan.rate)
    return offer.gain * horizon
end

function M.propose(snap, query)
    return { kind = ACTION.INVITE, name = party.offer(snap, query).name }
end

return M
