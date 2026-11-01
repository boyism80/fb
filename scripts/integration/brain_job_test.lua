local enlist = require("integration.scenarios.brain.job.enlist")

local HEALER_SHORT  = { WARRIOR = 25, ROGUE = 25, MAGE = 17, POET = 0 }
local WARRIOR_SHORT = { WARRIOR = 0, ROGUE = 30, MAGE = 20, POET = 40 }

test_suite {
    name      = "Brain Job Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return enlist.run(ctx, 0,
                { name = "healer_short", population = HEALER_SHORT, offering = 10, expected = "POET" })
        end,
        function(ctx)
            return enlist.run(ctx, 0,
                { name = "warrior_short", population = WARRIOR_SHORT, offering = 10, expected = "WARRIOR" })
        end,
        function(ctx)
            return enlist.run(ctx, 0,
                { name = "gather_offering", population = HEALER_SHORT, offering = 0, expected = "POET" })
        end,
        function(ctx)
            return enlist.run(ctx, 0,
                { name = "bag_full", population = HEALER_SHORT, offering = 0, filler = "목도", stuck = true })
        end,
    },
}
