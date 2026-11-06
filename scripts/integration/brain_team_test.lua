local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 300000

cases.start(16, 103)

test_suite {
    name      = "Brain Team Test",
    bot_count = 5,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx) return cases.group(ctx, { "warrior63", "poet65" }, 0, WATCH, 0) end },
                [2] = { function(ctx) return cases.group(ctx, { "rogue50", "poet65" }, 0, WATCH, 2) end },
                [4] = { function(ctx) return cases.group(ctx, { "mage60" }, 0, WATCH, 4) end },
            },
        },
    },
}
