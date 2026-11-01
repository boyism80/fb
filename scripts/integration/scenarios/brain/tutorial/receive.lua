local expect      = require("integration.lib.expect")
local script_step = require("integration.lib.script_step")

local M = {}

local TIMEOUT             = 15000
local QUEST_NAKRANG_INTRO = 224
local MAP                 = "낙랑의방"
local POSITION            = { 10, 11 }
local FIRST_SCROLL        = "낙랑의두루마리1"

function M.run(ctx, index)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:change_class("평민")
    script_step.run_script(bot, "test/integration", "set_quest", 1, QUEST_NAKRANG_INTRO, "none")
    local current = id2map(bot:map())
    if current == nil or current:name() ~= MAP then
        bot:map_move(MAP, POSITION[1], POSITION[2])
        local arrived
        arrived, bot = expect.arrive(ctx, index, MAP, TIMEOUT)
        if arrived == false then
            log("fatal", "[brain:tutorial:receive] FAILED arrive=false")
            return false
        end
    end

    local mind = brain.new()
    bot:brain(mind, { persona = { hunt = 1.0 }, seed = 1, halt = { task = "TUTORIAL", step = 1 } })
    local received = expect.within(ctx, TIMEOUT, function()
        local current  = ctx:bot(index)
        local progress = mind:progress(current)
        return current:has_item_by_name(FIRST_SCROLL) and progress ~= nil and progress.task == "TUTORIAL" and
            progress.step == 1
    end)
    expect.release(ctx, index, mind, TIMEOUT)
    bot = ctx:bot(index)

    if received then
        log("debug", "[brain:tutorial:receive] SUCCESS")
    else
        log("fatal",
            string.format("[brain:tutorial:receive] FAILED scroll=%s", tostring(bot:has_item_by_name(FIRST_SCROLL))))
    end
    return received
end

return M
