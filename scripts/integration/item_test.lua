local formation = require("integration.lib.formation")
local equipment = require("integration.scenarios.item.equipment")
local overflow  = require("integration.scenarios.item.equipment_overflow")
local combine   = require("integration.scenarios.item.combine")

local BOT_COUNT = 33

local function build_parallel(run_fn)
    local parallel = {}
    for i = 0, BOT_COUNT - 1 do
        parallel[i] = {
            function(ctx)
                return run_fn(ctx, i)
            end,
        }
    end
    return parallel
end

test_suite {
    name      = "Item Test",
    bot_count = BOT_COUNT,

    on_initialize = function(ctx)
        log("debug", "Item test initialized")
        formation.arrange_in_grid(ctx, "낙랑의방", 5, 5, 15, 13)
    end,

    on_parallel_scenario_finished = function(ctx, id)
        local bot = ctx:bot(id)
        bot:clear_all_drop_items()
        bot:clear_inventory()
        ctx:sleep(500)
        bot:money(0)
        bot:level(1)
        bot:str(1)
        bot:dex(1)
        bot:int(1)
        bot:gender("MALE")
        bot:base_hp(1000)
        bot:base_mp(1000)
    end,

    scenarios = {
        {
            parallel = build_parallel(equipment.run),
        },

        function(ctx)
            return overflow.run_overflow(ctx)
        end,

        {
            parallel = build_parallel(combine.run),
        },

        function(ctx)
            return combine.run_failure(ctx)
        end,
    },
}
