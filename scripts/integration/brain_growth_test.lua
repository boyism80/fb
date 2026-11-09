local enlist = require("integration.scenarios.brain.job.enlist")

test_suite {
    name      = "Brain Growth Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return enlist.growth(ctx, 0)
        end,
    },
}
