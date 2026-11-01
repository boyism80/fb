local blocked = require("integration.scenarios.brain.means.blocked")


test_suite {
    name      = "Brain Blocked Test",
    bot_count = 2,

    scenarios = {
        function(ctx)
            return blocked.run(ctx, 0, 1, { name = "one_gate", both = false })
        end,
        function(ctx)
            return blocked.run(ctx, 0, 1, { name = "both_gates", both = true })
        end,
    },
}
