local jump = require("integration.scenarios.brain.means.jump")

local WEST = { map = "국내성", x = 15, y = 107 }
local EAST = { map = "국내성", x = 206, y = 107, range = 2 }

test_suite {
    name      = "Brain Jump Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return jump.run(ctx, 0, { name = "walk", start = WEST, dest = EAST })
        end,
        function(ctx)
            return jump.run(ctx, 0,
                { name = "cardinal", start = WEST, dest = EAST, spells = { "비영사천문" }, mp = 100, cast = true })
        end,
        function(ctx)
            return jump.run(ctx, 0,
                { name = "cardinal_no_mp", start = WEST, dest = EAST, spells = { "비영사천문" }, mp = 0, cast = false })
        end,
        function(ctx)
            return jump.run(ctx, 0, {
                name  = "random_scroll",
                start = { map = "부여성", x = 8, y = 89 },
                dest  = { map = "주막왈숙네" },
                items = { "국내성비서" },
                use   = true,
            })
        end,
        function(ctx)
            return jump.run(ctx, 0, {
                name  = "fixed_scroll",
                start = WEST,
                dest  = { map = "장안성주막" },
                items = { "장안성비서" },
                use   = true,
            })
        end,
        function(ctx)
            return jump.run(ctx, 0,
                { name = "fixed_scroll_unneeded", start = WEST, dest = EAST, items = { "장안성비서" }, use = false })
        end,
    },
}
