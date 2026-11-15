local battle  = require("brain.policies.battle")
local tree    = require("brain.tree")
local guard   = require("brain.tasks.guard")
local flee    = require("brain.trees.flee")
local restore = require("brain.trees.restore")

local when, pick = tree.when, tree.pick

local M = {}

M.flee = flee.flee

M.notified = flee.notified

-- The guard (금강불체) is weighed beside flight and my own heal wherever my life is at stake: offered nowhere, a poet
-- that knew it never cast it once in 20,352 fights over 30 hours, while the fight forecast counted on it.
M.guard = tree.act("GUARD", guard.cast, "guard", guard.worth)

M.steady = pick({ M.flee, restore.restore }, tree.compared, "steady")

M.danger = when(function(snap)
    return snap.self.hp <= battle.burst(snap)
end, pick({ M.flee, M.guard, restore.recover }, tree.compared, "steady"), "danger")

return M
