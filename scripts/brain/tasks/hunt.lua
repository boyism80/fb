local hunt = require("brain.policies.hunt")

local M = {}

function M.run(brain, bot)
    local order = brain:recall(bot, "order")
    if order == nil then
        return true
    end

    local wanted = {}
    for _, name in ipairs(order.names) do
        wanted[name] = true
    end
    local hunted = hunt.go(brain, bot, {
        names  = order.names,
        wants  = function(name)
            return wanted[name] == true
        end,
        killed = function()
            order.killed = (order.killed or 0) + 1
            brain:remember(bot, "order", order)
        end,
        done   = function()
            return (order.killed or 0) >= order.count
        end,
    })
    if hunted and brain:active(bot) then
        brain:remember(bot, "order", nil)
    end
    return hunted
end

return M
