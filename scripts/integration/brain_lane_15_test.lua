local lanes = require("sim.world.lanes")

test_suite {
    name      = "Brain Lane 15 Test",
    bot_count = 0,

    scenarios = {
        function()
            return lanes.run(15)
        end,
    },
}
