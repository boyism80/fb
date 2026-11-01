local expect = require("integration.lib.expect")

local M = {}

local TIMEOUT = 600000
local STEPS   = 8

function M.run(ctx, index)
    local mind = brain.new()
    ctx:bot(index):brain(mind, { persona = { rush = 1 }, seed = 1 })
    local finished = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return mind:observe(current).class ~= "NONE"
    end)

    local bot     = ctx:bot(index)
    local reached = {}
    local lines   = {}
    for _, entry in ipairs(mind:states(bot) or {}) do
        if entry.task == "TUTORIAL" then
            reached[entry.step] = true
        end
        table.insert(lines, string.format("%s/%s/%d(%s)", entry.name, entry.task, entry.step, entry.reason))
    end
    local ordered = true
    for step = 0, STEPS do
        ordered = ordered and reached[step] == true
    end
    local seen    = mind:observe(bot)
    local landed  = id2map(bot:map())
    local summary = string.format("class=%s level=%d map=%s history=%s", seen and seen.class or "?", bot:level(),
        landed and landed:name() or "?", table.concat(lines, " -> "))
    expect.release(ctx, index, mind, TIMEOUT)

    local passed = finished and ordered
    if passed then
        log("debug", "[brain:tutorial:full] SUCCESS " .. summary)
    else
        log("fatal", string.format("[brain:tutorial:full] FAILED finished=%s ordered=%s %s", tostring(finished),
            tostring(ordered), summary))
    end
    return passed
end

return M
