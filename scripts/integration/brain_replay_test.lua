local replay = require("sim.replay")

test_suite {
    name      = "Brain Replay Test",
    bot_count = 0,

    scenarios = {
        function()
            return replay.run({ data = "sim.data.captured_1" })
        end,
    },
}
