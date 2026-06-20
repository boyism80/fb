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

test_suite {
    name      = "Skill Test",
    bot_count = 7,

    on_initialize = function(ctx)
        log("debug", "Skill test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        bot:level(5)
        bot:clear_all_spells()
        bot:clear_all_drop_items()
        ctx:sleep(skill.DEFAULT_INTERVAL)
    end,

    on_parallel_scenario_started = function(ctx, id)
        ctx:bot(id):level(5)
    end,

    on_parallel_scenario_finished = function(ctx, id)
        ctx:bot(id):clear_all_spells()
    end,

    scenarios = {
        {
            parallel = {
                [0] = {
                    function(ctx) return damage_near.run(ctx, 0) end,
                    function(ctx) return attack_cast.run(ctx, 0) end,
                    function(ctx) return multi_attack.run(ctx, 0) end,
                },
                [1] = {
                    function(ctx) return healing.run(ctx, 1) end,
                    function(ctx) return loot.run(ctx, 1) end,
                    function(ctx) return target.run(ctx, 1, 6) end,
                },
                [2] = {
                    function(ctx) return damage.run(ctx, 2) end,
                    function(ctx) return near_target.run(ctx, 2) end,
                },
                [3] = {
                    function(ctx) return buff.run(ctx, 3, 5) end,
                },
                [4] = {
                    function(ctx) return teleport.run(ctx, 4, 5) end,
                    function(ctx) return disguise.run(ctx, 4) end,
                    function(ctx) return shout.run(ctx, 4) end,
                },
            },
        },

        function(ctx)
            return healing_group.run(ctx, 0)
        end,

        function(ctx)
            return area.run(ctx, 0)
        end,

        function(ctx)
            return delay.run(ctx, 0)
        end,

        function(ctx)
            return special.run(ctx, 0)
        end,
    },
}
