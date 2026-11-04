local world = require("sim.world.suite")

test_suite {
    name      = "Brain World Probe Test",
    bot_count = 0,

    scenarios = {
        function()
            return world.run({
                data = "sim.data.snapshots_1",
                classes = { WARRIOR = true, ROGUE = true },
                band = 10,
                per = 2,
                only = { [10] = true, [16] = true, [37] = true },
                duration = 600000,
                rezen_multiplier = 20.0,
                exp_multiplier = 100.0,
                exp_limit = false,
                latency = 100,
                acts = true,
                output = "logs/brain-sim/probe",
            })
        end,
    },
}
