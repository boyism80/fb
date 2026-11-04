local cases = require("integration.scenarios.brain.state.cases")

local NAMES = { "warrior63", "rogue50", "mage60", "poet65", "mage35", "rogue20", "poet50", "warrior63nogg" }

cases.start(16, 103)

test_suite {
    name      = "Brain Sample Test",
    bot_count = #NAMES,

    scenarios = {
        function(ctx)
            return cases.sample(ctx, NAMES, 240000)
        end,
    },
}
