local MOVEMENT_STEPS   = 5
local DEFAULT_INTERVAL = 100

test_suite {
    name      = "Movement Test",
    bot_count = 5,

    scenarios = {
        function(ctx)
            log("debug", "Starting movement test")
            local bot = ctx:bot(ctx:bot_count() - 1)

            for _ = 1, MOVEMENT_STEPS do
                bot:move("BOTTOM", 1)
                ctx:sleep(DEFAULT_INTERVAL)
            end

            return true
        end,
    },
}
