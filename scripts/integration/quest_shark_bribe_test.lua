local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local check = qd.check

local function fail(fmt, ...)
    log("fatal", "quest_shark_bribe: " .. string.format(fmt, ...))
    return false
end

test_suite {
    name      = "Quest Shark Bribe Test",
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

            if check(bot, "verify_shark_bribe_once", "shark_bribe_once") == false then
                return fail("a shark general that was already bribed offered the bribe again")
            end
            return true
        end,
    },
}
