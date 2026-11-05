local cases = require("integration.scenarios.brain.state.cases")

test_suite {
    name      = "Brain Swarm Test",
    bot_count = 30,

    scenarios = {
        function(ctx)
            return cases.swarm(ctx, { "warrior63", "poet65", "rogue50", "mage60" }, 30, 180000)
        end,
    },
}
