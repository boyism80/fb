local lanes = require("sim.world.lanes")

test_suite {
    name      = "Brain Lane 19 Test",
    bot_count = 0,

    scenarios = {
        function(ctx)
            return lanes.run(19, ctx)
        end,
    },
}
