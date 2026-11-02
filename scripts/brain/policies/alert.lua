local M = {}

local ALARMING = { AGGRESSIVE = true }

function M.watch(brain, bot, snap)
    brain:concern(bot, { attacks = ALARMING, hp = snap.self.max_hp })
end

function M.reason(brain, bot, current)
    local first = require("brain.decide").first(brain, bot, current)
    return first ~= nil and first.task or nil
end

return M
