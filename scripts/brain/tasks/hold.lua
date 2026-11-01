local resp = require("integration.response")
local wait = require("brain.policies.wait")

local M = {}

function M.wanted(brain, bot, seen)
    return seen.ghost == false and seen.bag >= seen.capacity
end

function M.run(brain, bot)
    local seen = brain:observe(bot)
    wait.reply(bot, resp.item_remove, nil, function() return true end, seen.regen)
    return true
end

return M
