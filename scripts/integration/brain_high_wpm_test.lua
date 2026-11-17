local cases = require("integration.scenarios.brain.state.cases")

local WATCH = 600000

cases.start(16, 103)

test_suite {
    name      = "Brain High Warrior Poet Mage Test",
    bot_count = 3,

    scenarios = {
        function(ctx)
            return cases.forced_hunt(ctx, { "warrior3", "poet3", "mage3" }, "고균도절벽", WATCH)
        end,
    },
}
