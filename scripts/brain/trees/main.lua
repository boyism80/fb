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
local traffic  = require("brain.policies.traffic")

local choose, when, must, pick = tree.select, tree.when, tree.must, tree.pick

return choose({
    revive,
    when(function(snap)
        return snap.world.dialog == true
    end, must("DIALOG"), "dialog"),
    when(together.grouped, survive.notified),
    survive.danger,
    fight,
    -- My own step of a plan comes first unless a crewmate or I could die meanwhile: it is one step, and every bot of the
    -- plan waits on it. Weighed beside poisoning and resting, a mage put its step off for 38s while its rogue stood
    -- waiting (online 20261115-120942, 12:13:31-12:14:09; user 11-15). Waiting on another's step I do what is worth
    -- most from where I stand: ordered first, a poet waited 14s without one heal and its vanguard died (online 180725).
    when(traffic.planned, choose({
        when(traffic.stepping, pick({
            tree.act("TRAFFIC", traffic.follow, "traffic", traffic.worth),
            when(together.grouped, support.rescue),
            when(together.grouped, support.save),
            survive.flee,
        }, tree.compared, "step")),
        pick({
            tree.act("TRAFFIC", traffic.follow, "traffic", traffic.worth),
            when(together.grouped, support.rescue),
            when(together.grouped, support.tend),
            survive.steady,
        }, tree.compared, "traffic"),
    }), "planned"),
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
