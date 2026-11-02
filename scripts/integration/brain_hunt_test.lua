local cases = require("integration.scenarios.brain.state.cases")

cases.start(16, 100)

test_suite {
    name      = "Brain Hunt Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.field(ctx, 0)
        end,
        function(ctx)
            return cases.caster(ctx, 0)
        end,
    },
}
