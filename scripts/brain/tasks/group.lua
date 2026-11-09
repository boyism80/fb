local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")
local value  = require("brain.policies.value")
local learn  = require("brain.tasks.learn")
local outfit = require("brain.tasks.outfit")

local M = {}

local function postponed(snap, query, offer, task)
    local pending = task.pending(snap, query)
    if pending == nil then
        return false
    end
    local solo = value.baseline(snap, query).value
    local trip = solo > 0 and pending.trip / solo or 0
    return pending.gain * ground.horizon(snap, offer.plan.rate) > offer.plan.rate * trip
end

function M.offered(snap, query)
    return party.offer(snap, query) ~= nil
end

function M.free_now(snap, query)
    local offer = party.offer(snap, query)
    return postponed(snap, query, offer, learn) == false and postponed(snap, query, offer, outfit) == false
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
