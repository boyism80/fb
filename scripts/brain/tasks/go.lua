local travel = require("brain.policies.travel")

local M = {}

function M.run(brain, bot)
    local order = brain:recall(bot, "order")
    if order == nil then
        return true
    end

    local arrived, event = travel.go(brain, bot, order.dest)
    if arrived and brain:active(bot) then
        brain:remember(bot, "order", nil)
    end
    return arrived, event and event.type
end

return M
