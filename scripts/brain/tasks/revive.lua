local combat = require("brain.data.combat")
local near   = require("brain.policies.near")

local YES  = 1
local OATH = { YES, YES, YES, YES }

local M = {}

function M.wanted(snap)
    return snap.self.ghost
end

function M.propose(snap, query)
    local spot, move = near.approach(snap, query, combat.shrine.npcs)
    if spot ~= nil then
        return { kind = ACTION.TALK, npc = spot.name, answers = OATH, expect = { ghost = false } }
    end
    return move or { kind = ACTION.HOLD }
end

return M
