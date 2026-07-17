local lib      = require("integration.lib")

local SCRIPT           = "test/integration"
local DEFAULT_INTERVAL = 100

local function progress(bot, message)
    log("debug", message)
    bot:chat("=== " .. message .. " ===")
end

local function spawn_target(ctx, bot)
    local pos = bot:position()
    local mob = bot:spawn_monster("다람쥐", pos[1], pos[2] + 1)
    ctx:sleep(DEFAULT_INTERVAL)
    if mob.oid == 0 then
        return nil
    end
    return mob
end

local function run(bot, func, step, ...)
    return lib.script_step.run_script(bot, SCRIPT, func, step, ...)
end

test_suite {
    name      = "Resist Test",
    bot_count = 1,

    on_initialize = function(ctx)
        local bot = ctx:bot(0)
        lib.formation.arrange_in_line(ctx)
        bot:set_max_hp_mp(1000000, 1000000)
        bot:level(99)
        progress(bot, "RESIST TEST INITIALIZED")
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
        -- Scenario A: PHYSICAL damage resist (server-atomic)
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO A: PHYSICAL damage reduction")

            local mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_damage_resist", "phy_hi", mob.oid, "PHYSICAL", "1.0") == false then
                progress(bot, "FAILED: PHYSICAL 1.0")
                return false
            end

            run(bot, "cleanup", "mid_A")
            mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_damage_resist", "phy_lo", mob.oid, "PHYSICAL", "0.0") == false then
                progress(bot, "FAILED: PHYSICAL 0.0")
                return false
            end

            progress(bot, "SCENARIO A PASSED")
            return true
        end,

        -- Scenario B: MAGIC damage resist (server-atomic)
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO B: MAGIC damage reduction")

            local mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_damage_resist", "mag_hi", mob.oid, "MAGIC", "1.0") == false then
                progress(bot, "FAILED: MAGIC 1.0")
                return false
            end

            run(bot, "cleanup", "mid_B")
            mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_damage_resist", "mag_lo", mob.oid, "MAGIC", "0.0") == false then
                progress(bot, "FAILED: MAGIC 0.0")
                return false
            end

            progress(bot, "SCENARIO B PASSED")
            return true
        end,

        -- Scenario C: SPELL gate (server-atomic)
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO C: SPELL gate")

            local mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_spell_gate", "spell_hi", mob.oid, "1.0") == false then
                progress(bot, "FAILED: SPELL 1.0")
                return false
            end

            run(bot, "cleanup", "mid_C")
            mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_spell_gate", "spell_lo", mob.oid, "0.0") == false then
                progress(bot, "FAILED: SPELL 0.0")
                return false
            end

            progress(bot, "SCENARIO C PASSED")
            return true
        end,

        -- Scenario D: cross-type isolation (server-atomic)
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO D: cross-type isolation")

            local mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_cross_type", "cross", mob.oid) == false then
                progress(bot, "FAILED: cross-type")
                return false
            end

            progress(bot, "SCENARIO D PASSED")
            return true
        end,

        -- Scenario E: gate determinism
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO E: gate determinism")

            local types = { "SPELL", "MOVE", "PARALYSIS", "POISON", "DESPAIR", "CURSE", "BLIND" }
            for _, t in ipairs(types) do
                if run(bot, "verify_gate", "gate_hi_" .. t, t, "1.0") == false then
                    progress(bot, "FAILED: gate not always-resisted: " .. t)
                    return false
                end
                if run(bot, "verify_gate", "gate_lo_" .. t, t, "0.0") == false then
                    progress(bot, "FAILED: gate not never-resisted: " .. t)
                    return false
                end
            end

            progress(bot, "SCENARIO E PASSED")
            return true
        end,

        -- Scenario F: value clamp
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO F: value clamp")

            if run(bot, "verify_clamp", "clamp") == false then
                progress(bot, "FAILED: resist clamp")
                return false
            end

            progress(bot, "SCENARIO F PASSED")
            return true
        end,

        -- Scenario G: invalid inputs
        function(ctx)
            local bot = ctx:bot(0)
            progress(bot, "SCENARIO G: invalid input")

            if run(bot, "verify_damage_resist", "bad_oid", 0, "PHYSICAL", "1.0") == true then
                progress(bot, "FAILED: invalid oid unexpectedly succeeded")
                return false
            end

            local mob = spawn_target(ctx, bot)
            if mob == nil then return false end

            if run(bot, "verify_damage_resist", "bad_type", mob.oid, "BOGUS", "1.0") == true then
                progress(bot, "FAILED: unknown resist type unexpectedly succeeded")
                return false
            end

            progress(bot, "SCENARIO G PASSED")
            return true
        end,
    },
}
