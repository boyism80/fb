local tree     = require("brain.tree")
local defend   = require("brain.tasks.defend")
local chores   = require("brain.trees.chores")
local fight    = require("brain.trees.fight")
local hunt     = require("brain.trees.hunt")
local restore  = require("brain.trees.restore")
local revive   = require("brain.trees.revive")
local support  = require("brain.trees.support")
local survive  = require("brain.trees.survive")
local together = require("brain.trees.together")

local choose, when, must = tree.select, tree.when, tree.must

return choose({
    revive,
    when(function(snap)
        return snap.world.dialog == true
    end, must("DIALOG"), "dialog"),
    when(together.grouped, survive.notified),
    survive.danger,
    fight,
    when(together.grouped, support.rescue),
    when(defend.walled_in, when(defend.barred, tree.act("DEFEND", defend.clear_path, "clear_path")), "walled_in"),
    survive.steady,
    chores.upkeep,
    -- Servicing is done by each member on its own: nobody follows another member's errand. Following
    -- made the leader walk 4.5 minutes behind a member buying gear while neither hunted (RO+PO, 408-680s).
    -- Members meet again in front of the ground's entrance (hunt.lead_gate / follow_entry).
    chores.errand,
    chores.duties,
    chores.crew,
    when(together.grouped, together.make_way),
    hunt,
    restore.recuperate,
}, "main")
