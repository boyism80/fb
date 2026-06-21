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

local formation = lib.formation

local function slog(scope, fmt, ...)
    log("debug", string.format("[skill_test:%s] " .. fmt, scope, ...))
end

-- opts:
--   scope  (string)  log label for this parallel lane
--   name   (string)  scenario name for logs
--   run    (fn)      scenario entry point, e.g. healing.run
--   ctx    (table)   integration test context
--   causer (number)  bot index that casts / acts
--   target (number?) explicit target bot index
--   pair   (bool?)   when true, target = formation.skill_pair_target(causer)
local function run_scenario(opts)
    local scope  = opts.scope
    local name   = opts.name
    local run    = opts.run
    local ctx    = opts.ctx
    local causer = opts.causer

    local target = opts.target
    if target == nil and opts.pair then
        target = formation.skill_pair_target(causer)
    end

    if target ~= nil then
        slog(scope, "scenario=%s START causer=%d target=%d", name, causer, target)
    else
        slog(scope, "scenario=%s START causer=%d", name, causer)
    end

    local ok
    if target ~= nil then
        ok = run(ctx, causer, target)
    else
        ok = run(ctx, causer)
    end

    if ok == false then
        slog(scope, "scenario=%s FAILED (returned false)", name)
    else
        slog(scope, "scenario=%s DONE", name)
    end
    return ok
end

test_suite {
    name      = "Skill Test",
    bot_count = formation.SKILL_PARALLEL_BOT_COUNT,

    on_initialize = function(ctx)
        slog("init", "Skill test initialized bot_count=%d", ctx:bot_count())
        formation.arrange_skill_parallel(ctx)
    end,

    on_scenario_finished = function(ctx, scenario_index)
        if scenario_index == 0 then
            slog("cleanup", "parallel done - rearrange in line for sequential")
            formation.arrange_in_line(ctx)
            ctx:sleep(skill.DEFAULT_INTERVAL)
        end

        slog("cleanup", "on_scenario_finished begin scenario=%d", scenario_index)
        local bot = ctx:bot(0)
        bot:level(5)
        bot:clear_all_spells()
        bot:clear_all_drop_items()
        ctx:sleep(skill.DEFAULT_INTERVAL)
        slog("cleanup", "on_scenario_finished done scenario=%d", scenario_index)
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
                -- direction 1 (solo): bot 0 at (6, 6)
                [0] = {
                    function(ctx) return run_scenario({
                        scope = "bot0", name = "damage_near", run = damage_near.run,
                        ctx = ctx, causer = 0,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot0", name = "attack_cast", run = attack_cast.run,
                        ctx = ctx, causer = 0,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot0", name = "multi_attack", run = multi_attack.run,
                        ctx = ctx, causer = 0,
                    }) end,
                },
                -- direction 2 (pair): bot 1 causer, bot 2 target at (13, 6), (14, 6)
                [1] = {
                    function(ctx) return run_scenario({
                        scope = "bot1", name = "damage", run = damage.run,
                        ctx = ctx, causer = 1, pair = true,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot1", name = "target", run = target.run,
                        ctx = ctx, causer = 1, pair = true,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot1", name = "near_target", run = near_target.run,
                        ctx = ctx, causer = 1,
                    }) end,
                },
                -- direction 3 (pair): bot 3 causer, bot 4 target at (6, 12), (7, 12)
                [3] = {
                    function(ctx) return run_scenario({
                        scope = "bot3", name = "healing", run = healing.run,
                        ctx = ctx, causer = 3,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot3", name = "loot", run = loot.run,
                        ctx = ctx, causer = 3,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot3", name = "buff", run = buff.run,
                        ctx = ctx, causer = 3, pair = true,
                    }) end,
                },
                -- direction 4 (pair): bot 5 causer, bot 6 target at (13, 12), (14, 12)
                [5] = {
                    function(ctx) return run_scenario({
                        scope = "bot5", name = "teleport", run = teleport.run,
                        ctx = ctx, causer = 5, pair = true,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot5", name = "disguise", run = disguise.run,
                        ctx = ctx, causer = 5,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot5", name = "shout", run = shout.run,
                        ctx = ctx, causer = 5,
                    }) end,
                    function(ctx) return run_scenario({
                        scope = "bot5", name = "special", run = special.run,
                        ctx = ctx, causer = 5,
                    }) end,
                },
            },
        },

        function(ctx) return run_scenario({
            scope = "seq", name = "healing_group", run = healing_group.run,
            ctx = ctx, causer = 0,
        }) end,

        function(ctx) return run_scenario({
            scope = "seq", name = "area", run = area.run,
            ctx = ctx, causer = 0,
        }) end,

        function(ctx) return run_scenario({
            scope = "seq", name = "delay", run = delay.run,
            ctx = ctx, causer = 0,
        }) end,
    },
}
