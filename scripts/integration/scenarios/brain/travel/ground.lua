local expect      = require("integration.lib.expect")
local script_step = require("integration.lib.script_step")

local M = {}

local TIMEOUT        = 120000
local INN            = "주막왈숙네"
local POSITION       = { 3, 5 }
local QUEST_NAKRANG4 = 227
local SCROLL         = "낙랑의두루마리4"
local REWARD         = "낙랑의두루마리5"

function M.run(ctx, index)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:change_class("평민")
    bot:chat("/레벨바꾸기 1")
    script_step.run_script(bot, "test/integration", "set_quest", 1, QUEST_NAKRANG4, "none")
    local current = id2map(bot:map())
    if current == nil or current:name() ~= INN then
        bot:map_move(INN, POSITION[1], POSITION[2])
        local arrived
        arrived, bot = expect.arrive(ctx, index, INN, TIMEOUT)
        if arrived == false then
            log("fatal", "[brain:travel:ground] FAILED arrive=false")
            return false
        end
    end
    bot:create_item(SCROLL, 1)

    local mind = brain.new()
    bot:brain(mind, { persona = { class = 1 }, seed = 1, halt = { task = "TUTORIAL", step = 5 } })
    local passed = expect.within(ctx, TIMEOUT, function()
        return ctx:bot(index):has_item_by_name(REWARD)
    end)

    local lines = {}
    for _, entry in ipairs(mind:states(ctx:bot(index)) or {}) do
        table.insert(lines, string.format("%s/%s/%d(%s)", entry.name, entry.task, entry.step, entry.reason))
    end
    local landed = id2map(ctx:bot(index):map())
    local summary = string.format("map=%s history=%s", landed and landed:name() or "?", table.concat(lines, " -> "))
    expect.release(ctx, index, mind, TIMEOUT)

    if passed then
        log("debug", "[brain:travel:ground] SUCCESS " .. summary)
    else
        log("fatal", "[brain:travel:ground] FAILED " .. summary)
    end
    return passed
end

return M
