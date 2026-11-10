local tree    = require("brain.tree")
local restore = require("brain.trees.restore")
local support = require("brain.trees.support")
local upkeep  = require("brain.trees.upkeep")

local function gainful(snap, facts, found)
    local worth = tree.compared(snap, facts, found)
    return worth ~= nil and worth > 0 and worth or nil
end

return tree.pick({
    support.care,
    restore.restore,
    upkeep.loot,
}, gainful, "idle")
