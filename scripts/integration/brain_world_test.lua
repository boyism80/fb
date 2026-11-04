local world = require("sim.world.suite")

test_suite {
    name      = "Brain World Test",
    bot_count = 0,

    scenarios = {
        function()
            return world.run(require("sim.world.roster").solo)
        end,
    },
}
