local cases = require("integration.scenarios.brain.state.cases")

cases.start(16, 103)

test_suite {
    name      = "Brain Ground Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.grounds(ctx, 0)
        end,
        function(ctx)
            return cases.profiles(ctx, 0)
        end,
    },
}
