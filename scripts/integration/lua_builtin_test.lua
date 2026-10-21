local script_step = require("integration.lib.script_step")

local function fail(message)
    log("fatal", "Lua builtin test: " .. message)
    return false
end

test_suite {
    name      = "Lua Builtin Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            for i = 1, 2 do
                local step = "builtin_exception_" .. i
                if script_step.run_script(bot, "test/integration", "builtin_exception", step) ~= true then
                    return fail("the builtin exception did not reach the script as a Lua error at try " .. i)
                end
            end
            return true
        end,
    },
}
