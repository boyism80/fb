local cases = require("integration.scenarios.brain.state.cases")

test_suite {
    name      = "Brain State Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.idle(ctx, 0)
        end,
        function(ctx)
            return cases.equip(ctx, 0)
        end,
        function(ctx)
            return cases.unreachable(ctx, 0)
        end,
        function(ctx)
            return cases.reattach(ctx, 0)
        end,
    },
}
