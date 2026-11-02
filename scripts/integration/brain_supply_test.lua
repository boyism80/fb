local cases = require("integration.scenarios.brain.state.cases")

cases.start(49, 160)

test_suite {
    name      = "Brain Supply Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.supplier(ctx, 0)
        end,
        function(ctx)
            return cases.seller(ctx, 0)
        end,
    },
}
