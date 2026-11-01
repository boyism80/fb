local danger = require("brain.policies.danger")

local M = {}

local ALARMING = { AGGRESSIVE = true }

function M.watch(brain, bot)
    local limit = danger.threat(brain, bot)
    for _, mob in ipairs(brain:mobs(bot)) do
        if ALARMING[mob.attack] and mob.damage ~= nil then
            limit = math.max(limit, danger.cost(brain, bot, mob) or mob.damage)
        end
    end
    brain:concern(bot, { attacks = ALARMING, hp = limit })
end

function M.reason(brain, bot, current)
    M.watch(brain, bot)
    local first = require("brain.decide").first(brain, bot, current)
    return first ~= nil and first.task or nil
end

return M
