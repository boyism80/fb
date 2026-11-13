local report = require("sim.report")

test_suite {
    name      = "Brain Sim Report Test",
    bot_count = 0,

    scenarios = {
        function()
            report.collect(assert(os.getenv("FB_RUN_DIR"), "the report needs FB_RUN_DIR"))
            return true
        end,
    },
}
