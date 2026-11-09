local tree = require("brain.tree")
local aid  = require("brain.tasks.aid")

local act, pick = tree.act, tree.pick

local M = {}

M.rescue = act("AID", aid.urgent, "rescue", aid.worth("urgent"))

M.care = pick({
    act("AID", aid.offer("heal"), "heal", aid.worth("heal")),
    act("AID", aid.offer("debuff"), "debuff", aid.worth("debuff")),
    act("AID", aid.offer("ward"), "ward", aid.worth("ward")),
}, tree.compared, "care")

return M
