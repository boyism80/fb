-- A storage stack may grow past the item capacity and must keep its full count across relogin.
-- Retrieving more than the capacity into a new slot is rejected.
local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local ACORN          = "도토리"
local ACORN_CAPACITY = 201

local check = qd.check

local function fail(fmt, ...)
    log("fatal", "storage_stack: " .. string.format(fmt, ...))
    return false
end

local function drain(bot)
    bot:clear_inventory()
    return check(bot, "drain_stored", "drain", ACORN, ACORN_CAPACITY)
end

test_suite {
    name      = "Storage Stack Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        drain(ctx:bot(0))
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if drain(bot) == false then
                return fail("could not empty the stored %s", ACORN)
            end

            for i = 1, 2 do
                bot:create_item(ACORN, ACORN_CAPACITY)
                if check(bot, "store_item", "store_" .. i, ACORN, ACORN_CAPACITY) == false then
                    return fail("could not store %s x%d (round %d)", ACORN, ACORN_CAPACITY, i)
                end
            end
            if check(bot, "verify_stored", "stored", ACORN, ACORN_CAPACITY * 2) == false then
                return fail("storage did not merge both stacks")
            end

            if bot:logout() == false then
                return fail("logout")
            end
            if bot:login() == false then
                return fail("relogin")
            end
            bot = ctx:bot(0)
            lib.formation.arrange_in_line(ctx)

            if check(bot, "verify_stored", "relogin_stored", ACORN, ACORN_CAPACITY * 2) == false then
                return fail("relogin clamped the storage stack")
            end

            if check(bot, "retrieve_item", "over_capacity", ACORN, ACORN_CAPACITY + 1, "fail") == false then
                return fail("retrieved more than the capacity into one slot")
            end
            if check(bot, "retrieve_item", "retrieve", ACORN, ACORN_CAPACITY, "ok") == false
                or check(bot, "verify_items", "retrieved_items", string.format("%s=%d", ACORN, ACORN_CAPACITY)) == false
                or check(bot, "verify_stored", "remaining", ACORN, ACORN_CAPACITY) == false then
                return fail("retrieving one full stack broke")
            end
            return true
        end,
    },
}
