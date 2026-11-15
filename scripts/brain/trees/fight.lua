local tree     = require("brain.tree")
local defend   = require("brain.tasks.defend")
local restore  = require("brain.trees.restore")
local support  = require("brain.trees.support")
local survive  = require("brain.trees.survive")
local together = require("brain.trees.together")
local rest     = require("brain.tasks.rest")

local choose, when, act, pick = tree.select, tree.when, tree.act, tree.pick

local function steady(snap, facts)
    return rest.low(snap, facts) == false
end

return when(defend.attacked, when(defend.holds_ground, choose({
    when(together.grouped, choose({
        support.rescue,
        when(steady, when(together.parted, when(together.on_ground, when(together.can_move, together.rejoin)))),
    })),
    pick({
        survive.flee,
        restore.recover,
        support.care,
        act("FIELD", together.step_off, "step_off", together.step_worth),
        act("DEFEND", defend.strike_back, "attack", defend.value),
    }, tree.compared, "weigh"),
    -- With nothing above worth doing, what is left for a hurt bot beside its healer is the wait for a heal, and the
    -- blows land whether it strikes back or stands: a warrior at 1,884 of 5,607 hp, whose own fight with one fox
    -- weighed as a loss beside its stronger mage, stood under three for 28s (offline WA+MA seed 5, 580-610s, run
    -- 04:58; user: struck and doing nothing is a bug).
    when(restore.awaiting, act("DEFEND", defend.strike_back, "strike_awaiting")),
})), "attacked")
