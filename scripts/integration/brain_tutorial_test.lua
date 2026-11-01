local full = require("integration.scenarios.brain.tutorial.full")

test_suite {
    name      = "Brain Tutorial Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return full.run(ctx, 0)
        end,
    },
}
