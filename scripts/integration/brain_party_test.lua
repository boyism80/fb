local cases = require("integration.scenarios.brain.state.cases")

cases.start(16, 103)

test_suite {
    name      = "Brain Party Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.parties(ctx, 0)
        end,
    },
}
