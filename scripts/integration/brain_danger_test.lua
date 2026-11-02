local cases = require("integration.scenarios.brain.danger.cases")

local GROUND = "고구려왕초보사냥1"

local function anywhere()
    return true
end

local function away(bot)
    local model = id2map(bot:map())
    return model ~= nil and model:name() ~= GROUND
end

test_suite {
    name      = "Brain Danger Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return cases.run(ctx, 0, {
                name     = "hit_on_way",
                hp       = 80,
                monster  = "도삭산까만소",
                start    = { x = 3, y = 3 },
                offset   = { 6, 14 },
                order    = { task = "GO", dest = { map = "국내성", x = 206, y = 107, range = 2 } },
                tasks    = { "ORDER" },
                finished = away,
                keep     = true,
            })
        end,
        function(ctx)
            return cases.run(ctx, 0, {
                name     = "rest_first",
                hp       = 2,
                order    = { task = "HUNT", names = { "다람쥐" }, count = 2 },
                tasks    = { "ORDER" },
                finished = anywhere,
            })
        end,
        function(ctx)
            return cases.run(ctx, 0, {
                name     = "escape_walk",
                hp       = 44,
                monster  = "도삭산까만소",
                finished = away,
            })
        end,
        function(ctx)
            return cases.run(ctx, 0, {
                name     = "escape_scroll",
                hp       = 44,
                monster  = "도삭산까만소",
                items    = { "노란비서" },
                tasks    = { "ESCAPE" },
                jump     = "노란비서",
                finished = away,
            })
        end,
    },
}
