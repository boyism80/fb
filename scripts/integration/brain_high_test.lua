local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000

cases.start(16, 103)

test_suite {
    name      = "Brain High Test",
    bot_count = 5,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx) return cases.group(ctx, { "warrior3", "rogue3", "poet3" }, 0, WATCH, 0) end },
                [3] = { function(ctx) return cases.group(ctx, { "mage3", "mage3b" }, 0, WATCH, 3) end },
            },
        },
    },
}
