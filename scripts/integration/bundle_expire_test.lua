local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local ACORN       = "도토리"
local WOODEN      = "목도"
local EXPIRE_WAIT = 15000

local check = qd.check

local function fail(fmt, ...)
    log("fatal", "bundle_expire: " .. string.format(fmt, ...))
    return false
end

test_suite {
    name      = "Bundle Expire Test",
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

            if check(bot, "make_items", "timed_bundle", ACORN, 10, 1) == false
                or check(bot, "make_items", "permanent_bundle", ACORN, 5, 0) == false
                or check(bot, "make_items", "timed_single", WOODEN, 1, 1) == false then
                return fail("could not create the items")
            end

            ctx:sleep(EXPIRE_WAIT)

            if check(bot, "verify_items", "after_expire", string.format("%s=15,%s=0", ACORN, WOODEN)) == false then
                return fail("a bundle kept a duration or a timed single item did not expire")
            end
            return true
        end,
    },
}
