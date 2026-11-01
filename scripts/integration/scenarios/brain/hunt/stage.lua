local expect      = require("integration.lib.expect")
local script_step = require("integration.lib.script_step")

local M = {}

local GROUND   = "고구려왕초보사냥1"
local POSITION = { 9, 18 }
local WEAPON   = "초심자의목도"

function M.run(ctx, index, case)
    local bot = ctx:bot(index)
    bot:clear_inventory()
    bot:change_class("평민")
    bot:chat(string.format("/레벨바꾸기 %d", case.level))
    script_step.run_script(bot, "test/integration", "set_quest", 1, case.quest, "none")
    local current = id2map(bot:map())
    if current == nil or current:name() ~= GROUND then
        bot:map_move(GROUND, POSITION[1], POSITION[2])
        local arrived
        arrived, bot = expect.arrive(ctx, index, GROUND, case.timeout)
        if arrived == false then
            log("fatal", string.format("[brain:hunt:%s] FAILED arrive=false", case.name))
            return false
        end
    end
    bot:create_item(WEAPON, 1)
    bot:equip(bot:item_slot(WEAPON))
    bot:create_item(case.scroll, 1)

    local mind = brain.new()
    bot:brain(mind, { persona = { class = 1 }, seed = 1, halt = { task = "TUTORIAL", step = case.halt } })
    local passed = expect.within(ctx, case.timeout, function()
        return ctx:bot(index):has_item_by_name(case.reward)
    end)

    local lines = {}
    for _, entry in ipairs(mind:states(ctx:bot(index)) or {}) do
        table.insert(lines, string.format("%s/%s/%d(%s)", entry.name, entry.task, entry.step, entry.reason))
    end
    local summary = string.format("level=%d history=%s", ctx:bot(index):level(), table.concat(lines, " -> "))
    expect.release(ctx, index, mind, case.timeout)

    if passed then
        log("debug", string.format("[brain:hunt:%s] SUCCESS %s", case.name, summary))
    else
        log("fatal", string.format("[brain:hunt:%s] FAILED %s", case.name, summary))
    end
    return passed
end

return M
