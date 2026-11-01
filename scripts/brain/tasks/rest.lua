local resp   = require("integration.response")
local danger = require("brain.policies.danger")
local wait   = require("brain.policies.wait")

local M = {}

function M.goal(brain, bot)
    local seen   = brain:observe(bot)
    local threat = danger.threat(brain, bot)
    local fight  = 0
    for _, mob in ipairs(brain:mobs(bot)) do
        if mob.damage ~= nil and danger.hits(mob) then
            fight = math.max(fight, danger.cost(brain, bot, mob) or mob.damage)
        end
    end
    return math.min(seen.max_hp, threat + fight)
end

function M.needed(brain, bot)
    local threat = danger.threat(brain, bot)
    return threat > 0 and brain:observe(bot).hp <= threat
end

function M.wanted(brain, bot, seen)
    return seen.ghost == false and M.needed(brain, bot)
end

function M.run(brain, bot)
    local goal = M.goal(brain, bot)
    while brain:active(bot) and brain:observe(bot).hp < goal and danger.fatal(brain, bot) == nil do
        wait.reply(bot, resp.update, nil, function() return true end, brain:observe(bot).regen)
    end
    return true
end

return M
