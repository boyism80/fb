local moments = require("sim.moments")

test_suite {
    name      = "Brain Moments Test",
    bot_count = 0,

    scenarios = {
        function()
            local shard, shards = string.match(os.getenv("MOMENTS_SHARD") or "0/1", "^(%d+)/(%d+)$")
            return moments.replay({
                list   = os.getenv("MOMENTS_LIST"),
                out    = os.getenv("MOMENTS_OUT"),
                patch  = os.getenv("MOMENTS_PATCH"),
                row    = tonumber(os.getenv("MOMENTS_ROW")),
                stride = tonumber(os.getenv("MOMENTS_STRIDE")),
                shard  = tonumber(shard),
                shards = tonumber(shards),
            })
        end,
    },
}
