local script_step = require("integration.lib.script_step")

test_suite({
    name = "State Machine Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if script_step.run_script(bot, "test/integration", "verify_state_machine", "state_machine") ~= true then
                log("fatal", "State machine test: every, call or cancel did not behave on the machine thread")
                return false
            end
            return true
        end,
    },
})
