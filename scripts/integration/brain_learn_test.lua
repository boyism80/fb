local cases = require("integration.scenarios.brain.state.cases")

cases.start(16, 103)

test_suite {
    name      = "Brain Learn Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.learn(ctx, 0, "warrior63", "사중공격", {
                { name = "도토리", count = 80 },
            }, 2000)
        end,
    },
}
