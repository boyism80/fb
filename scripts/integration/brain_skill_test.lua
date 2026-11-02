local cases = require("integration.scenarios.brain.state.cases")

cases.start(17, 107)

test_suite {
    name      = "Brain Skill Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.buffer(ctx, 0)
        end,
        function(ctx)
            return cases.striker(ctx, 0)
        end,
    },
}
