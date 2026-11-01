local expect = require("integration.lib.expect")

local M = {}

local TIMEOUT = 5000

function M.run(ctx, index)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:change_class("평민")

    local mind = brain.new()
    bot:brain(mind, { persona = { hunt = 1.0 }, seed = 1, halt = { task = "TUTORIAL", step = 0 } })
    local reached = expect.within(ctx, TIMEOUT, function()
        local state    = mind:state(bot)
        local progress = mind:progress(bot)
        return state ~= nil and state.name == "PROGRESS" and progress ~= nil and progress.task == "TUTORIAL"
    end)
    local lines = {}
    for _, entry in ipairs(mind:states(bot)) do
        table.insert(lines, string.format("%s/%s/%d(%s)", entry.name, entry.task, entry.step, entry.reason))
    end
    local history = table.concat(lines, " -> ")
    bot:brain(nil)

    if reached then
        log("debug", "[brain:initial] SUCCESS " .. history)
    else
        log("fatal", "[brain:initial] FAILED expected PROGRESS/TUTORIAL, got " .. history)
    end
    return reached
end

return M
