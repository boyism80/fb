local M = {}

M.upkeep = require("brain.trees.upkeep").tree
M.errand = require("brain.trees.errand")
M.duties = require("brain.trees.duties")
M.crew   = require("brain.trees.crew")

return M
