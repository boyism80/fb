local lanes = require("sim.world.lanes")

test_suite {
    name      = "Brain Lane 24 Test",
    bot_count = 0,

    scenarios = {
        function()
            return lanes.run(24)
        end,
    },
}
