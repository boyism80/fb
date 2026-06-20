local lib = require("integration.lib")

test_suite {
    name      = "Lib Smoke Test",
    bot_count = 2,

    scenarios = {
        function(ctx)
            log("debug", "Lib smoke test scenario running")
            lib.formation.arrange_in_line(ctx)
            return ctx:bot_count() == 2
        end,
    },
}
