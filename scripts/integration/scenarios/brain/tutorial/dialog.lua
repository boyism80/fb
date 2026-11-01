local expect      = require("integration.lib.expect")
local script_step = require("integration.lib.script_step")

local M = {}

local TIMEOUT = 30000
local QUESTS  = { 225, 226, 227, 228, 229 }

local CASES = {
    {
        name   = "inn",
        level  = 1,
        items  = { { "낙랑의두루마리1", 1 } },
        reward = "낙랑의두루마리2",
        halt   = 2,
        moved  = true,
    },
    {
        name   = "dotol",
        level  = 1,
        items  = { { "낙랑의두루마리2", 1 }, { "도톨", 2 } },
        reward = "낙랑의두루마리3",
        halt   = 3,
    },
    {
        name   = "weapon",
        level  = 1,
        items  = { { "낙랑의두루마리3", 1 }, { "초심자의목도", 1 } },
        reward = "낙랑의두루마리4",
        halt   = 4,
    },
    {
        name   = "quiz",
        level  = 3,
        items  = { { "낙랑의두루마리6", 1 } },
        reward = "낙랑의두루마리7",
        halt   = 7,
    },
    {
        name     = "job",
        level    = 5,
        items    = { { "낙랑의두루마리8", 1 } },
        consumed = "낙랑의두루마리8",
    },
}

function M.run(ctx, index)
    local ok = true
    for _, case in ipairs(CASES) do
        local bot = ctx:bot(index)
        bot:clear_inventory()
        bot:change_class("평민")
        bot:chat(string.format("/레벨바꾸기 %d", case.level))
        for _, id in ipairs(QUESTS) do
            script_step.run_script(bot, "test/integration", "set_quest", 1, id, "none")
        end
        for _, item in ipairs(case.items) do
            bot:create_item(item[1], item[2])
        end

        local map  = bot:map()
        local mind = brain.new()
        local halt = case.halt and { task = "TUTORIAL", step = case.halt } or nil
        bot:brain(mind, { persona = { class = 1 }, seed = 1, halt = halt })
        local passed = expect.within(ctx, TIMEOUT, function()
            local current = ctx:bot(index)
            if case.moved or case.reward == nil then
                if current:map() == map then
                    return false
                end
            end
            if case.reward ~= nil then
                return current:has_item_by_name(case.reward)
            end
            return current:has_item_by_name(case.consumed) == false
        end)
        expect.release(ctx, index, mind, TIMEOUT)

        if passed then
            log("debug", "[brain:tutorial:" .. case.name .. "] SUCCESS")
        else
            log("fatal", "[brain:tutorial:" .. case.name .. "] FAILED")
        end
        ok = ok and passed
    end
    return ok
end

return M
