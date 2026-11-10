local data    = require("brain.data.combat")
local profile = require("brain.profile")

local M = {}

local ALARMING = {}
for name, attack in pairs(data.attacks) do
    if attack.seeks then
        ALARMING[name] = true
    end
end

-- A crewmate taking a blow in sight decides again during a long action, the same as my own hit: a healer
-- walking 12s behind its warrior kept walking while the warrior died in its sight (WA+PO 세작, 534-546s).
function M.watch(brain, bot, snap)
    local crew = {}
    for _, name in ipairs(snap.group or {}) do
        crew[name] = true
    end
    brain:concern(bot, { attacks = ALARMING, hp = snap.self.max_hp, crew = crew })
end

function M.reason(brain, bot, current, kind)
    local owner   = bot:name()
    local started = profile.start(owner)
    local first   = require("brain.decide").first(brain, bot, current, kind)
    profile.finish(brain, bot, owner, "alarm:" .. tostring(kind), started)
    return first ~= nil and first.task or nil
end

return M
