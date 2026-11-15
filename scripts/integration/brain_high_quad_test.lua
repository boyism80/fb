local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000
local FORM  = 480000

cases.start(16, 103)

test_suite {
    name      = "Brain High Quad Test",
    bot_count = 10,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx)
                    return cases.group(ctx, { "warrior99", "rogue99", "poet99" }, 0, WATCH, 0, FORM, true)
                end },
                [3] = { function(ctx)
                    return cases.group(ctx, { "warrior99", "mage99", "poet99" }, 0, WATCH, 3, FORM, true)
                end },
                [6] = { function(ctx)
                    return cases.group(ctx, { "warrior99", "mage99b" }, 0, WATCH, 6, FORM, true)
                end },
                [8] = { function(ctx)
                    return cases.group(ctx, { "mage99", "mage99b" }, 0, WATCH, 8, FORM, true)
                end },
            },
        },
    },
}
