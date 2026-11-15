local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000
local FORM  = 480000

cases.start(16, 103)

test_suite {
    name      = "Brain High Warrior Poet Mage Test",
    bot_count = 3,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx)
                    return cases.group(ctx, { "warrior99", "poet99", "mage99" }, 0, WATCH, 0, FORM, true)
                end },
            },
        },
    },
}
