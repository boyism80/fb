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
                    return cases.group(ctx, { "warrior93", "rogue92", "poet94" }, 0, WATCH, 0, FORM, true)
                end },
                [3] = { function(ctx)
                    return cases.group(ctx, { "warrior93", "mage95", "poet94" }, 0, WATCH, 3, FORM, true)
                end },
                [6] = { function(ctx)
                    return cases.group(ctx, { "warrior93", "mage91" }, 0, WATCH, 6, FORM, true)
                end },
                [8] = { function(ctx)
                    return cases.group(ctx, { "mage95", "mage91" }, 0, WATCH, 8, FORM, true)
                end },
            },
        },
    },
}
