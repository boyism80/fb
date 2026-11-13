local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000

cases.start(16, 103)

test_suite {
    name      = "Brain High Test",
    bot_count = 5,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx) return cases.group(ctx, { "warrior93", "rogue92", "poet94" }, 0, WATCH, 0) end },
                [3] = { function(ctx) return cases.group(ctx, { "mage95", "mage91" }, 0, WATCH, 3) end },
            },
        },
    },
}
