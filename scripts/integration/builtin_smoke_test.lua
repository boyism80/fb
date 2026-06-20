test_suite {
    name      = "Builtin Smoke Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            log("debug", "Builtin smoke test scenario running")
            local bot = ctx:bot(0)

            bot:level(77)
            bot:money(10000)

            return bot:level() == 77 and bot:money() == 10000
        end,
    },
}
