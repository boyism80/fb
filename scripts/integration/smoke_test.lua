test_suite {
    name      = "Smoke Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:money(10000)
            log("debug", "Smoke test scenario running")
            ctx:sleep(500)
            return true
        end,
    },
}
