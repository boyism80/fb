local script_step = require("integration.lib.script_step")

local M = {}

function M.run(ctx)
    local bot = ctx:bot(0)
    bot:clear_inventory()
    return script_step.run_script(bot, "test/integration", "verify_castle_strip", "CASTLE_STRIP") == true
end

return M
