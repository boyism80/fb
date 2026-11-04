local probe = require("sim.world.probe")

test_suite {
    name      = "Brain World Ground Probe Test",
    bot_count = 0,

    scenarios = {
        function()
            return probe.gates({
                data = "sim.data.snapshots_1",
                sets = {
                    { class = "WARRIOR", low = 83, high = 89, bundles = { "국내비밀세작의집", "국내성흉가" } },
                    { class = "ROGUE", low = 83, high = 89, bundles = { "국내비밀세작의집", "국내성흉가" } },
                    { class = "WARRIOR", low = 90, high = 99, bundles = { "국내비밀세작의집", "국내성흉가" } },
                    { class = "ROGUE", low = 90, high = 99, bundles = { "국내비밀세작의집", "국내성흉가" } },
                },
            })
        end,
    },
}
