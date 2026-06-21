local lib         = require("integration.lib")
local script_step = require("integration.lib.script_step")

local SCRIPT_FILE = "scripts/integration.lua"

local function run_step(ctx, bot, step, ...)
    local bot_name = bot:name()
    local arg_list = { ... }
    local args_str = #arg_list > 0 and table.concat(arg_list, ", ") or "-"
    log("debug", "Clan: " ..string.format("running step '%s' on bot '%s' (args: %s)", step, bot_name, args_str))

    local parts = { "run", step }
    for i = 1, select("#", ...) do
        parts[#parts + 1] = select(i, ...)
    end

    local ok, response = script_step.run_script(bot, SCRIPT_FILE, table.unpack(parts))
    if ok then
        log("debug", "Clan: " ..string.format("step '%s' PASSED on bot '%s'", step, bot_name))
    else
        log("debug", "Clan: " ..string.format("step '%s' FAILED on bot '%s', response=%q", step, bot_name, response or ""))
    end
    return ok
end

test_suite {
    name      = "Clan Test",
    bot_count = 4,

    on_initialize = function(ctx)
        log("debug", "Clan: " .."test initialized, arranging bots")
        lib.formation.arrange_in_line(ctx)
        log("debug", "Clan: " .."formation complete")
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            log("debug", "Clan: " ..string.format(
                "bot[%d] name=%s oid=%d",
                i,
                bot:name(),
                bot:oid()))
        end
    end,

    scenarios = {
        function(ctx)
            log("debug", "Clan: " .."scenario 1 (clan creation) started")
            local bot0 = ctx:bot(0)
            if run_step(ctx, bot0, "create_clan") == false then
                log("debug", "Clan: " .."scenario 1 FAILED at create_clan")
                return false
            end
            log("debug", "Clan: " .."scenario 1 completed successfully")
            return true
        end,

        function(ctx)
            log("debug", "Clan: " .."scenario 2 (clan title) started")
            local bot0 = ctx:bot(0)
            if run_step(ctx, bot0, "set_title") == false then
                log("debug", "Clan: " .."scenario 2 FAILED at set_title")
                return false
            end
            log("debug", "Clan: " .."scenario 2 completed successfully")
            return true
        end,

        function(ctx)
            log("debug", "Clan: " .."scenario 3 (clan invite) started")
            local bot0 = ctx:bot(0)
            local bot1 = ctx:bot(1)
            if run_step(ctx, bot0, "invite", bot1:name()) == false then
                log("debug", "Clan: " .."scenario 3 FAILED at invite")
                return false
            end
            if run_step(ctx, bot1, "verify_member", bot0:name()) == false then
                log("debug", "Clan: " .."scenario 3 FAILED at verify_member")
                return false
            end
            log("debug", "Clan: " .."scenario 3 completed successfully")
            return true
        end,

        function(ctx)
            log("debug", "Clan: " .."scenario 4 (clan role) started")
            local bot0 = ctx:bot(0)
            local bot1 = ctx:bot(1)
            local bot2 = ctx:bot(2)
            local bot3 = ctx:bot(3)
            if run_step(ctx, bot1, "invite_fail", bot2:name()) == false then
                log("debug", "Clan: " .."scenario 4 FAILED at invite_fail")
                return false
            end
            if run_step(ctx, bot0, "promote_deputy", bot1:name()) == false then
                log("debug", "Clan: " .."scenario 4 FAILED at promote_deputy")
                return false
            end
            if run_step(ctx, bot1, "invite", bot2:name()) == false then
                log("debug", "Clan: " .."scenario 4 FAILED at invite")
                return false
            end
            if run_step(ctx, bot1, "change_role_fail", bot2:name(), "3") == false then
                log("debug", "Clan: " .."scenario 4 FAILED at change_role_fail (deputy)")
                return false
            end
            if run_step(ctx, bot1, "change_role_fail", bot3:name(), "0") == false then
                log("debug", "Clan: " .."scenario 4 FAILED at change_role_fail (non-member)")
                return false
            end
            log("debug", "Clan: " .."scenario 4 completed successfully")
            return true
        end,

        function(ctx)
            log("debug", "Clan: " .."scenario 5 (clan title change) started")
            local bot0 = ctx:bot(0)
            local bot1 = ctx:bot(1)
            local bot2 = ctx:bot(2)
            if run_step(ctx, bot2, "change_title_fail", "MateTitle") == false then
                log("debug", "Clan: " .."scenario 5 FAILED at change_title_fail (mate)")
                return false
            end
            if run_step(ctx, bot1, "change_title_fail", "DeputyTitle") == false then
                log("debug", "Clan: " .."scenario 5 FAILED at change_title_fail (deputy)")
                return false
            end
            if run_step(ctx, bot0, "change_title", "MasterTitle") == false then
                log("debug", "Clan: " .."scenario 5 FAILED at change_title")
                return false
            end
            log("debug", "Clan: " .."scenario 5 completed successfully")
            return true
        end,

        function(ctx)
            log("debug", "Clan: " .."scenario 6 (clan disbanding) started")
            local bot0 = ctx:bot(0)
            local bot1 = ctx:bot(1)
            local bot2 = ctx:bot(2)
            if run_step(ctx, bot2, "kick_fail", bot1:name()) == false then
                log("debug", "Clan: " .."scenario 6 FAILED at kick_fail")
                return false
            end
            if run_step(ctx, bot2, "leave") == false then
                log("debug", "Clan: " .."scenario 6 FAILED at leave")
                return false
            end
            if run_step(ctx, bot0, "kick", bot1:name()) == false then
                log("debug", "Clan: " .."scenario 6 FAILED at kick")
                return false
            end
            if run_step(ctx, bot0, "destroy_clan") == false then
                log("debug", "Clan: " .."scenario 6 FAILED at destroy_clan")
                return false
            end
            log("debug", "Clan: " .."scenario 6 completed successfully")
            return true
        end,
    },
}
