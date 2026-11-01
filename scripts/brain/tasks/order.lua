local M = {}

function M.wanted(brain, bot, seen)
    return seen.ghost == false and brain:recall(bot, "order") ~= nil
end

function M.run(brain, bot)
    local order = brain:recall(bot, "order")
    if order == nil then
        return true
    end
    return require("brain.tasks." .. string.lower(order.task)).run(brain, bot)
end

return M
