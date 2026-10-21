local script_step = require("integration.lib.script_step")

local M = {}

function M.run(ctx)
    local bot = ctx:bot(0)
    bot:clear_inventory()
    if script_step.run_script(bot, "test/integration", "reward_split", "REWARD_SPLIT") ~= true then
        log("fatal", "reward split: a reward over one slot was not paid in full")
        return false
    end
    bot:clear_inventory()
    return true
end

return M
