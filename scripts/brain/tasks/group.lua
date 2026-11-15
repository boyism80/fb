local ground = require("brain.policies.ground")
local party  = require("brain.policies.party")
local value  = require("brain.policies.value")
local learn  = require("brain.tasks.learn")
local outfit = require("brain.tasks.outfit")

local M = {}

-- An errand put off is done on the next servicing, so putting it off loses its gain for what is left of the lap; going
-- first puts the group off for the trip. Weighed until the next level, which never comes at 99, any gear put the
-- invite off for good (offline 20261115-104514).
local function postponed(snap, query, offer, task)
    local pending = task.pending(snap, query)
    if pending == nil then
        return false
    end
    local solo = value.baseline(snap, query).value
    local trip = solo > 0 and pending.trip / solo or 0
    return pending.gain * ground.lap_left(snap, party.plan(snap, query)) > offer.plan.rate * trip
end

function M.offered(snap, query)
    return party.offer(snap, query) ~= nil
end

function M.free_now(snap, query)
    local offer = party.offer(snap, query)
    return postponed(snap, query, offer, learn) == false and postponed(snap, query, offer, outfit) == false
end

function M.value(snap, query)
    local offer = party.offer(snap, query)
    return offer.gain * ground.lap(offer.plan)
end

function M.propose(snap, query)
    return { kind = ACTION.INVITE, name = party.offer(snap, query).name }
end

return M
