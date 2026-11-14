local traffic = require("brain.policies.traffic")

local M = {}

M.propose = traffic.follow

return M
