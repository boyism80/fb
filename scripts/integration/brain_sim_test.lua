local static = require("sim.static")

test_suite {
    name      = "Brain Sim Test",
    bot_count = 0,

    scenarios = {
        function()
            return static.run({
                data = "sim.data.snapshots_1",
                per = 10,
                teams = 5,
                output = "../../scripts/sim/data/static_1.lua"
            })
        end,
    },
}
