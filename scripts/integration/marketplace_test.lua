local lib         = require("integration.lib")
local script_step = require("integration.lib.script_step")

local SCRIPT_FILE = "scripts/integration.lua"

local function run_step(bot, role, step)
    return script_step.run_script_with_prefix(
        bot,
        SCRIPT_FILE,
        "MP:PASS:" .. step,
        "MP:FAIL:" .. step,
        role,
        step)
end

local function cleanup_bot(bot)
    bot:chat("/아이템초기화")
    bot:chat("/아이템삭제")
    bot:money(0)
end

test_suite {
    name      = "Marketplace Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Marketplace test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        cleanup_bot(ctx:bot(0))
        cleanup_bot(ctx:bot(1))
        ctx:sleep(1000)
    end,

    scenarios = {
        function(ctx)
            log("debug", "Starting marketplace test")
            local bot_a = ctx:bot(0)
            local bot_b = ctx:bot(1)

            cleanup_bot(bot_a)
            cleanup_bot(bot_b)

            if run_step(bot_a, "run_a", "verify_empty") == false then return false end
            if run_step(bot_a, "run_a", "prepare_weapon") == false then return false end
            if run_step(bot_b, "run_b", "prepare_bundle") == false then return false end
            if run_step(bot_a, "run_a", "list_weapon_fee_fail") == false then return false end
            if run_step(bot_a, "run_a", "list_weapon_ok") == false then return false end
            if run_step(bot_a, "run_a", "verify_weapon_count") == false then return false end
            if run_step(bot_b, "run_b", "list_bundle") == false then return false end
            if run_step(bot_a, "run_a", "verify_two_listings") == false then return false end
            if run_step(bot_a, "run_a", "cancel_relist") == false then return false end
            if run_step(bot_b, "run_b", "purchase_weapon") == false then return false end
            if run_step(bot_a, "run_a", "purchase_bundle") == false then return false end
            if run_step(bot_a, "run_a", "verify_empty_final") == false then return false end

            log("debug", "MARKETPLACE TEST COMPLETED SUCCESSFULLY")
            return true
        end,
    },
}
