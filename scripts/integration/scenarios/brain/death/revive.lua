local expect = require("integration.lib.expect")
local ghost  = require("integration.scenarios.brain.stage.ghost")

local M = {}

local TIMEOUT = 120000
local SLOT    = 9

local function stage(ctx, index, case)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:clear_all_spells()
    bot:change_class("전사")
    for _, name in ipairs(case.spells or {}) do
        bot:learn_spell(name)
    end

    local current = id2map(bot:map())
    bot:map_move(case.start.map, case.start.x, case.start.y)
    if current == nil or current:name() ~= case.start.map then
        local arrived = expect.arrive(ctx, index, case.start.map, TIMEOUT)
        if arrived == false then
            return nil
        end
    end
    ctx:bot(index):map_move(case.start.map, case.start.x, case.start.y)
    if ghost.die(ctx, index, { slot = SLOT }) == false then
        return nil
    end
    return ctx:bot(index)
end

function M.run(ctx, index, case)
    local bot = stage(ctx, index, case)
    if bot == nil then
        log("fatal", string.format("[brain:death:%s] FAILED stage", case.name))
        return false
    end

    local mind = brain.new()
    bot:brain(mind, { seed = 1 })
    local revived = expect.settle(ctx, index, mind, TIMEOUT, function(current)
        return current:state() ~= "GHOST"
    end)

    bot          = ctx:bot(index)
    local landed = id2map(bot:map())
    local jumped = mind:observe(bot).jumped
    local lines  = {}
    for _, entry in ipairs(mind:states(bot) or {}) do
        table.insert(lines, string.format("%s/%s(%s)", entry.name, entry.task, entry.reason))
    end
    expect.release(ctx, index, mind, TIMEOUT)

    local passed  = revived and jumped == case.jump
    local summary = string.format("revived=%s map=%s jumped=%s history=%s", tostring(revived),
        landed and landed:name() or "?", tostring(jumped), table.concat(lines, " -> "))
    if passed then
        log("debug", string.format("[brain:death:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:death:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
