local lanes = require("sim.world.lanes")

test_suite {
    name      = "Brain Lane 13 Test",
    bot_count = 0,

    scenarios = {
        function()
            return lanes.run(13)
        end,
    },
}
