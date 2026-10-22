local script_step = require("integration.lib.script_step")

local ACHIEVEMENT_ID = 9001

test_suite {
    name      = "Achievement Ref Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            if script_step.run_script(a, "test/integration", "verify_achievement_replaced", "achievement_replaced", tostring(ACHIEVEMENT_ID)) ~= true then
                log("fatal", "Achievement ref test: a script kept reading an achievement after it was replaced")
                return false
            end
            return true
        end,
    },
}
