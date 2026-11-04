local lanes = require("sim.world.lanes")

test_suite {
    name      = "Brain Lane 05 Test",
    bot_count = 0,

    scenarios = {
        function()
            return lanes.run(5)
        end,
    },
}
