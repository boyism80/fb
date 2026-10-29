local script_step = require("integration.lib.script_step")

local BUNDLE = "도토리"
local SINGLE = "목도"

test_suite {
    name      = "Storage Grant Limit Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if script_step.run_script(bot, "test/integration", "verify_storage_grant_limit", "storage_grant_limit", BUNDLE, SINGLE) ~= true then
                log("fatal", "Storage grant limit test: a storage box that no character can receive was accepted")
                return false
            end
            return true
        end,
    },
}
