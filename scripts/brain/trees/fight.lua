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
        when(steady, when(together.apart, when(together.on_ground, when(together.can_move, together.regroup)))),
    })),
    pick({
        survive.flee,
        restore,
        support.care,
        act("DEFEND", defend.strike_back, "attack", defend.value),
    }, tree.compared, "weigh"),
})), "attacked")
