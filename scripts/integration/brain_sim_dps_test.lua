local static = require("sim.static")

test_suite {
    name      = "Brain Sim Dps Test",
    bot_count = 0,

    scenarios = {
        function()
            return static.dps({
                data = "sim.data.snapshots_1",
                per = 10,
                requires = { WARRIOR = "건곤대나이", ROGUE = "필살검무" }
            })
        end,
    },
}
