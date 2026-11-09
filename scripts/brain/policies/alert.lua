local M = {}

local ALARMING = { AGGRESSIVE = true }

-- A crewmate taking a blow in sight decides again during a long action, the same as my own hit: a healer
-- walking 12s behind its warrior kept walking while the warrior died in its sight (WA+PO 세작, 534-546s).
function M.watch(brain, bot, snap)
    local crew = {}
    for _, name in ipairs(snap.group or {}) do
        crew[name] = true
    end
    brain:concern(bot, { attacks = ALARMING, hp = snap.self.max_hp, crew = crew })
end

function M.reason(brain, bot, current)
    local first = require("brain.decide").first(brain, bot, current)
    return first ~= nil and first.task or nil
end

return M
