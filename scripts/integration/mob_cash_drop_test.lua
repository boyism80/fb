local script_step = require("integration.lib.script_step")

local MAP_NAME   = "가상계"
local MAP_POS    = { 7, 7 }
local MOB_NAME   = "자생원"
local KILL_COUNT = 20
local DROP_WAIT  = 1000

local function fail(message)
    log("fatal", "Mob cash drop test: " .. message)
    return false
end

test_suite {
    name      = "Mob Cash Drop Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:map_move(MAP_NAME, MAP_POS[1], MAP_POS[2], ctx:suite_slot())

            if script_step.run_script(bot, "test/integration", "kill_spawned", "mob_cash_kill", MOB_NAME, tostring(KILL_COUNT)) ~= true then
                return fail("could not kill " .. MOB_NAME)
            end
            ctx:sleep(DROP_WAIT)

            local pass, text = script_step.run_script(bot, "test/integration", "verify_dropped_cash", "mob_cash_verify",
                "은전", "엽전")
            if pass ~= true then
                return fail("a 50 coin drop did not land as 은전: " .. tostring(text))
            end
            return true
        end,
    },
}
