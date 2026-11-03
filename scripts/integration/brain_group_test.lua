local cases = require("integration.scenarios.brain.state.cases")

cases.start(16, 103)

test_suite {
    name      = "Brain Group Test",
    bot_count = 2,

    scenarios = {
        function(ctx)
            return cases.group(ctx, { "warrior63", "poet65" }, 2)
        end,
    },
}
