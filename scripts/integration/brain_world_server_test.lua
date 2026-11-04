local world = require("sim.world.suite")

test_suite {
    name      = "Brain World Server Test",
    bot_count = 0,

    scenarios = {
        function()
            return world.run({
                data = "sim.data.captured_2",
                classes = { WARRIOR = true, ROGUE = true },
                start = true,
                duration = 240000,
                rezen_multiplier = 20.0,
                exp_multiplier = 100.0,
                exp_limit = false,
                latency = 100,
                output = "logs/brain-sim/server",
            })
        end,
    },
}
