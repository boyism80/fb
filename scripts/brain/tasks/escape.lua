local danger   = require("brain.policies.danger")
local teleport = require("brain.data.teleport")
local travel   = require("brain.policies.travel")

local M = {}

function M.wanted(brain, bot, seen)
    return seen.ghost == false and danger.fatal(brain, bot) ~= nil
end

function M.run(brain, bot)
    local refuges = teleport.returns[brain:observe(bot).nation]
    if refuges == nil then
        return false
    end
    local arrived, event = travel.go(brain, bot, { maps = refuges })
    return arrived, event and event.type
end

return M
