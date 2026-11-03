local teleport = require("brain.data.teleport")
local danger   = require("brain.policies.danger")
local value    = require("brain.policies.value")

local M = {}

function M.wanted(snap, query)
    return snap.self.ghost == false and danger.fatal(snap, query) ~= nil and teleport.returns[snap.self.nation] ~= nil
end

function M.propose(snap)
    return {
        kind = ACTION.MOVE,
        dest = { maps = teleport.returns[snap.self.nation] },
        forget = snap.record.death ~= nil
    }
end

function M.value(snap, query)
    local threat = danger.fatal(snap, query)
    local worth  = value.baseline(snap, query)
    local stay   = danger.chance(snap, query, threat, math.huge) * danger.loss(snap, query, worth)
    local trip   = value.trip(snap, query, M.propose(snap, query).dest, worth)
    return stay - (trip ~= nil and trip.cost or math.huge)
end

return M
