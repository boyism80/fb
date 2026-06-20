local lib      = require("integration.lib")
local skill    = require("integration.lib.skill")

local damage_near   = require("integration.scenarios.skill.damage_near")
local attack_cast   = require("integration.scenarios.skill.attack_cast")
local multi_attack  = require("integration.scenarios.skill.multi_attack")
local healing       = require("integration.scenarios.skill.healing")
local loot          = require("integration.scenarios.skill.loot")
local target        = require("integration.scenarios.skill.target")
local damage        = require("integration.scenarios.skill.damage")
local near_target   = require("integration.scenarios.skill.near_target")
local buff          = require("integration.scenarios.skill.buff")
local teleport      = require("integration.scenarios.skill.teleport")
local disguise      = require("integration.scenarios.skill.disguise")
local shout         = require("integration.scenarios.skill.shout")
local healing_group = require("integration.scenarios.skill.healing_group")
local area          = require("integration.scenarios.skill.area")
local delay         = require("integration.scenarios.skill.delay")
local special       = require("integration.scenarios.skill.special")

local function slog(scope, fmt, ...)
    log("debug", string.format("[skill_test:%s] " .. fmt, scope, ...))
end

local function run_scenario(scope, name, fn, ...)
    slog(scope, "scenario=%s START", name)
    local ok = fn(...)
    if ok == false then
        slog(scope, "scenario=%s FAILED (returned false)", name)
    else
        slog(scope, "scenario=%s DONE", name)
    end
    return ok
end

test_suite {
    name      = "Skill Test",
    bot_count = 8,

    on_initialize = function(ctx)
        slog("init", "Skill test initialized bot_count=%d", ctx:bot_count())
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        slog("cleanup", "on_scenario_finished begin")
        local bot = ctx:bot(0)
        bot:level(5)
        bot:clear_all_spells()
        bot:clear_all_drop_items()
        ctx:sleep(skill.DEFAULT_INTERVAL)
        slog("cleanup", "on_scenario_finished done")
    end,

    on_parallel_scenario_started = function(ctx, id)
        slog("parallel", "bot=%d lane START", id)
        ctx:bot(id):level(5)
    end,

    on_parallel_scenario_finished = function(ctx, id)
        slog("parallel", "bot=%d lane DONE", id)
        ctx:bot(id):clear_all_spells()
    end,

    scenarios = {
        {
            parallel = {
                -- [0] = {
                --     function(ctx) return run_scenario("bot0", "damage_near", damage_near.run, ctx, 0) end,
                --     function(ctx) return run_scenario("bot0", "attack_cast", attack_cast.run, ctx, 0) end,
                --     function(ctx) return run_scenario("bot0", "multi_attack", multi_attack.run, ctx, 0) end,
                -- },
                [1] = {
                    function(ctx) return run_scenario("bot1", "healing", healing.run, ctx, 1) end,
                    function(ctx) return run_scenario("bot1", "loot", loot.run, ctx, 1) end,
                    function(ctx) return run_scenario("bot1", "target", target.run, ctx, 1, target.TARGET_BOT_INDEX) end,
                },
                -- [2] = {
                --     function(ctx) return run_scenario("bot2", "damage", damage.run, ctx, 2) end,
                --     function(ctx) return run_scenario("bot2", "near_target", near_target.run, ctx, 2) end,
                -- },
                [3] = {
                    function(ctx) return run_scenario("bot3", "buff", buff.run, ctx, 3, 5) end,
                },
                -- [4] = {
                --     function(ctx) return run_scenario("bot4", "teleport", teleport.run, ctx, 4, 5) end,
                --     function(ctx) return run_scenario("bot4", "disguise", disguise.run, ctx, 4) end,
                --     function(ctx) return run_scenario("bot4", "shout", shout.run, ctx, 4) end,
                -- },
            },
        },

        -- function(ctx)
        --     slog("seq", "phase=1 healing_group START")
        --     return run_scenario("seq", "healing_group", healing_group.run, ctx, 0)
        -- end,

        -- function(ctx)
        --     slog("seq", "phase=2 area START")
        --     return run_scenario("seq", "area", area.run, ctx, 0)
        -- end,

        -- function(ctx)
        --     slog("seq", "phase=3 delay START")
        --     return run_scenario("seq", "delay", delay.run, ctx, 0)
        -- end,

        function(ctx)
            slog("seq", "phase=4 special START")
            return run_scenario("seq", "special", special.run, ctx, 0)
        end,
    },
}
