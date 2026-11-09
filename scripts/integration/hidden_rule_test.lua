local lib         = require("integration.lib")
local script_step = require("integration.lib.script_step")

local ROLE_USER  = 0
local ROLE_ADMIN = 2

local function fail(message)
    log("fatal", "Hidden rule test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

test_suite {
    name      = "Hidden Rule Test",
    bot_count = 3,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_finished = function(ctx)
        run_step(ctx:bot(2), "set_role", "restore_viewer", ctx:bot(0):name(), tostring(ROLE_ADMIN))
    end,

    scenarios = {
        function(ctx)
            local viewer = ctx:bot(0)
            local target = ctx:bot(1)
            local runner = ctx:bot(2)

            viewer:direction("RIGHT")
            if run_step(runner, "set_role", "demote_viewer", viewer:name(), tostring(ROLE_USER)) == false then
                return fail("could not demote the viewer")
            end
            if run_step(runner, "verify_hidden_rule", "hidden_rule", viewer:name(), target:name()) == false then
                return fail("a visible operator must block the viewer and a super-hidden one must not")
            end
            return true
        end,
    },
}
