local revive = require("integration.scenarios.brain.death.revive")

local TOWN   = { map = "국내성", x = 15, y = 107 }
local GROUND = { map = "고구려왕초보사냥1", x = 9, y = 18 }
local ANY    = "*"

test_suite {
    name      = "Brain Revive Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return revive.run(ctx, 0, { name = "town_walk", start = TOWN })
        end,
        function(ctx)
            return revive.run(ctx, 0, { name = "ground_walk", start = GROUND })
        end,
        function(ctx)
            return revive.run(ctx, 0, { name = "ground_spell", start = GROUND, spells = { "성황령" }, jump = "성황령" })
        end,
    },
}
