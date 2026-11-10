local tree     = require("brain.tree")
local aid      = require("brain.tasks.aid")
local together = require("brain.trees.together")

local act, pick, when = tree.act, tree.pick, tree.when

local M = {}

M.rescue = act("AID", aid.urgent, "rescue", aid.worth("urgent"))

M.care = pick({
    act("AID", aid.offer("heal"), "heal", aid.worth("heal")),
    act("AID", aid.offer("debuff"), "debuff", aid.worth("debuff")),
    act("AID", aid.offer("ward"), "ward", aid.worth("ward")),
    when(together.supporting, act("FIELD", together.close, "close", together.close_worth)),
}, tree.compared, "care")

return M
