local battle  = require("brain.policies.battle")
local tree    = require("brain.tree")
local flee    = require("brain.trees.flee")
local restore = require("brain.trees.restore")

local when, pick = tree.when, tree.pick

local M = {}

M.flee = flee.flee

M.notified = flee.notified

M.steady = pick({ M.flee, restore.restore }, tree.compared, "steady")

M.danger = when(function(snap)
    return snap.self.hp <= battle.burst(snap)
end, M.steady, "danger")

return M
