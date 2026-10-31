local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local check = qd.check

local KEPT_ITEM     = "콩"
local EXPIRING_ITEM = "조"
local CLOCK_WAIT_MS = 1500
local TIMER_WAIT_MS = 12000

local function fail(fmt, ...)
    log("fatal", "ground_lifetime: " .. string.format(fmt, ...))
    return false
end

test_suite {
    name      = "Ground Lifetime Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        check(bot, "clear_ground_items", "ground_clear")
        bot:chat("/현재시간초기화")
        ctx:sleep(CLOCK_WAIT_MS)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            check(bot, "clear_ground_items", "ground_clear_before")
            if check(bot, "spawn_ground_items", "ground_spawn", KEPT_ITEM, EXPIRING_ITEM) == false then
                return fail("could not place the ground items")
            end

            bot:chat("/시간가속 00:04:00")
            ctx:sleep(TIMER_WAIT_MS)

            if check(bot, "verify_ground_items", "ground_verify", KEPT_ITEM, EXPIRING_ITEM) == false then
                return fail("an item placed without lifetime expired or a dropped item stayed past its lifetime")
            end
            return true
        end,
    },
}
