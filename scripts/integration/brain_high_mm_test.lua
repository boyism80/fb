local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000
local FORM  = 480000

cases.start(16, 103)

test_suite {
    name      = "Brain High Mage Pair Test",
    bot_count = 2,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx)
                    return cases.group(ctx, { "mage95", "mage91" }, 0, WATCH, 0, FORM, true)
                end },
            },
        },
    },
}
