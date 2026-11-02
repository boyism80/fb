local script_step = require("integration.lib.script_step")

local M = {}

function M.inside(bot)
    return script_step.run_script(bot, "test/integration", "verify_map_instance", "instance", "1") == true
end

return M
