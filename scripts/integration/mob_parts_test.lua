local lib = require("integration.lib")

local SCRIPT           = "test/integration"
local DEFAULT_INTERVAL = 150

local function progress(bot, message)
    log("debug", message)
    bot:chat("=== " .. message .. " ===")
end

local function spawn_assembly(ctx, bot)
    local pos  = bot:position()
    local body = bot:spawn_monster("다람쥐", pos[1], pos[2] + 1)
    local p1   = bot:spawn_monster("토끼", pos[1] + 1, pos[2] + 1)
    local p2   = bot:spawn_monster("토끼", pos[1] + 2, pos[2] + 1)
    ctx:sleep(DEFAULT_INTERVAL)

    if body.oid == 0 or p1.oid == 0 or p2.oid == 0 then
        return nil
    end
    return body, p1, p2
end

local function run(bot, func, step, ...)
    return lib.script_step.run_script(bot, SCRIPT, func, step, ...)
end

test_suite {
    name      = "Mob Parts Test",
    bot_count = 1,

    on_initialize = function(ctx)
        local bot = ctx:bot(0)
        lib.formation.arrange_in_line(ctx)
        bot:set_max_hp_mp(1000000, 1000000)
        bot:level(99)
        progress(bot, "MOB PARTS TEST INITIALIZED")
    end,

    on_scenario_started = function(ctx)
        local bot = ctx:bot(0)
        run(bot, "cleanup", "cleanup_start")
        bot:clear_all_drop_items()
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        run(bot, "cleanup", "cleanup_end")
        bot:clear_all_drop_items()
    end,

    scenarios = {
        -- Scenario A: PARTS mode routing + death (server-atomic)
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO A: PARTS routing")

            local body, p1, p2 = spawn_assembly(ctx, bot)
            if body == nil then
                progress(bot, "FAILED: spawn assembly")
                return false
            end

            if run(bot, "link_parts", "link_A", body.oid, p1.oid, p2.oid, "PARTS") == false then
                progress(bot, "FAILED: link_parts PARTS")
                return false
            end
            if run(bot, "verify_parts_hit", "hit_A", body.oid, p1.oid, "PARTS") == false then
                progress(bot, "FAILED: PARTS hit routing")
                return false
            end
            if run(bot, "verify_body_direct_ignored", "direct_A", body.oid) == false then
                progress(bot, "FAILED: body direct hit")
                return false
            end
            if run(bot, "verify_parts_death", "death_A", body.oid, p1.oid, p2.oid, "PARTS") == false then
                progress(bot, "FAILED: PARTS death cascade")
                return false
            end

            progress(bot, "SCENARIO A PASSED")
            return true
        end,

        -- Scenario B: BODY mode routing + death (server-atomic)
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO B: BODY routing")

            local body, p1, p2 = spawn_assembly(ctx, bot)
            if body == nil then
                progress(bot, "FAILED: spawn assembly")
                return false
            end

            if run(bot, "link_parts", "link_B", body.oid, p1.oid, p2.oid, "BODY") == false then
                progress(bot, "FAILED: link_parts BODY")
                return false
            end
            if run(bot, "verify_parts_hit", "hit_B", body.oid, p1.oid, "BODY") == false then
                progress(bot, "FAILED: BODY hit routing")
                return false
            end
            if run(bot, "verify_body_direct_ignored", "direct_B", body.oid) == false then
                progress(bot, "FAILED: body direct hit")
                return false
            end
            if run(bot, "verify_parts_death", "death_B", body.oid, p1.oid, p2.oid, "BODY") == false then
                progress(bot, "FAILED: BODY death")
                return false
            end

            progress(bot, "SCENARIO B PASSED")
            return true
        end,

        -- Scenario C: invalid add_part links are rejected
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO C: link rejection")

            local cases = { "self", "already_linked", "nested", "part_as_body" }
            for _, case in ipairs(cases) do
                if run(bot, "verify_link_reject", "reject_" .. case, case) == false then
                    progress(bot, "FAILED: reject case not rejected: " .. case)
                    return false
                end
            end

            progress(bot, "SCENARIO C PASSED")
            return true
        end,

        -- Scenario D: linking a non-existent oid fails cleanly
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO D: invalid oid link")

            if run(bot, "link_parts", "link_invalid", 0, 0, 0, "PARTS") == true then
                progress(bot, "FAILED: invalid oid link unexpectedly succeeded")
                return false
            end

            progress(bot, "SCENARIO D PASSED")
            return true
        end,
    },
}
