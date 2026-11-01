local initial = require("integration.scenarios.brain.state.initial")
local receive = require("integration.scenarios.brain.tutorial.receive")
local command = require("integration.scenarios.brain.guard.command")
local dialog  = require("integration.scenarios.brain.tutorial.dialog")
local ground  = require("integration.scenarios.brain.travel.ground")
local hunt    = require("integration.scenarios.brain.hunt.stage")

test_suite {
    name      = "Brain Scenario Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            return initial.run(ctx, 0)
        end,
        function(ctx)
            return receive.run(ctx, 0)
        end,
        function(ctx)
            return command.run(ctx, 0)
        end,
        function(ctx)
            return dialog.run(ctx, 0)
        end,
        function(ctx)
            return ground.run(ctx, 0)
        end,
        function(ctx)
            return hunt.run(ctx, 0, {
                name    = "prey",
                level   = 1,
                quest   = 228,
                scroll  = "낙랑의두루마리5",
                reward  = "낙랑의두루마리6",
                halt    = 6,
                timeout = 240000,
            })
        end,
        function(ctx)
            return hunt.run(ctx, 0, {
                name    = "grow",
                level   = 3,
                quest   = 229,
                scroll  = "낙랑의두루마리7",
                reward  = "낙랑의두루마리8",
                halt    = 8,
                timeout = 300000,
            })
        end,
    },
}
