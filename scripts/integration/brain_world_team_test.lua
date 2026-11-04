local world = require("sim.world.suite")

test_suite {
    name      = "Brain World Team Test",
    bot_count = 0,

    scenarios = {
        function()
            return world.teams(require("sim.world.roster").team)
        end,
    },
}
