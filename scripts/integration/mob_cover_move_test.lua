local lib = require("integration.lib")
local script_step = require("integration.lib.script_step")

local MOB_NAME = "다람쥐"

local function fail(message)
    log("fatal", "Mob cover move test: " .. message)
    return false
end

test_suite({
    name = "Mob Cover Move Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            local pass, text = script_step.run_script(bot, "test/integration", "verify_cover_move", "mob_cover_move",
                MOB_NAME)
            if pass ~= true then
                return fail("a mob moved against the cover rule: " .. tostring(text))
            end
            return true
        end,
    },
})
