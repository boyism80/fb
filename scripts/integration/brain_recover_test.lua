local recover = require("integration.scenarios.brain.death.recover")

local TOWN   = { map = "국내성", x = 15, y = 107 }
local GROUND = { map = "고구려왕초보사냥1", x = 9, y = 18 }
local ANY    = "*"

test_suite {
    name      = "Brain Recover Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return recover.run(ctx, 0, { name = "walk_back" })
        end,
        function(ctx)
            return recover.run(ctx, 0, { name = "cardinal_back", spells = { "비영사천문" }, jump = ANY })
        end,
        function(ctx)
            return recover.run(ctx, 0, { name = "vanished", vanish = true, money = 0 })
        end,
        function(ctx)
            return recover.run(ctx, 0, { name = "guarded", guard = "도삭산까만소", money = 0 })
        end,
    },
}
