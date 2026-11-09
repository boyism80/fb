local combat = require("brain.data.combat")
local near   = require("brain.policies.near")
local tree   = require("brain.tree")

local choose, when, act = tree.select, tree.when, tree.act

local OATH = { 1, 1, 1, 1 }

local function oath(snap, facts)
    local spot = near.approach(snap, facts, combat.shrine.npcs)
    if spot == nil then
        return nil
    end
    return { kind = ACTION.TALK, npc = spot.name, answers = OATH, expect = { ghost = false } }
end

local function walk(snap, facts)
    local _, move = near.approach(snap, facts, combat.shrine.npcs)
    return move or { kind = ACTION.HOLD }
end

return when(function(snap)
    return snap.self.ghost
end, choose({
    act("REVIVE", oath, "oath"),
    act("REVIVE", walk, "to_shrine"),
}), "revive")
