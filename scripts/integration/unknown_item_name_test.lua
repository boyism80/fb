local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local ACORN   = "도토리"
local UNKNOWN = "integration_unknown_item"

local check = qd.check

local function fail(fmt, ...)
    log("fatal", "unknown_item_name: " .. string.format(fmt, ...))
    return false
end

test_suite {
    name      = "Unknown Item Name Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        ctx:bot(0):clear_inventory()
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()
            bot:create_item(ACORN, 5)

            if check(bot, "verify_unknown_item_name", "table_forms", ACORN) == false then
                return fail("a table form mkitem or rmitem skipped an unknown name and succeeded")
            end

            if check(bot, "exchange_items", "unknown_cost", UNKNOWN .. "=1", ACORN .. "=1", "LACK_COST") == false
                or check(bot, "verify_items", "unknown_cost_items", ACORN .. "=5") == false then
                return fail("an exchange with an unknown cost item gave the reward")
            end

            if check(bot, "exchange_items", "unknown_reward", ACORN .. "=1", UNKNOWN .. "=1", "LACK_CAPACITY") == false
                or check(bot, "verify_items", "unknown_reward_items", ACORN .. "=5") == false then
                return fail("an exchange with an unknown reward item took the cost")
            end
            return true
        end,
    },
}
