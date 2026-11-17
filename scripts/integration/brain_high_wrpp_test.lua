local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000
local FORM  = 480000

cases.start(28, 17, "흉노족1")

test_suite {
    name      = "Brain High Warrior Rogue Poet Poet Test",
    bot_count = 4,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx)
                    return cases.group(ctx, { "warrior3", "rogue3", "poet3", "poet3b" }, 0, WATCH, 0, FORM, true)
                end },
            },
        },
    },
}
