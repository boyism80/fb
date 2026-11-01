local M = {}

local HITTING = { AGGRESSIVE = true, RETALIATE = true, STATIONARY = true }

function M.hits(mob)
    return HITTING[mob.attack] == true
end

function M.learn(brain, bot, mob, swings)
    if swings == nil or swings <= 0 or mob.hp == nil then
        return
    end
    local strike  = brain:recall(bot, "strike") or { hp = 0, swings = 0 }
    strike.hp     = strike.hp + mob.hp
    strike.swings = strike.swings + swings
    brain:remember(bot, "strike", strike)
end

function M.cost(brain, bot, mob)
    if M.hits(mob) == false then
        return 0
    end
    local strike = brain:recall(bot, "strike")
    local swing  = brain:observe(bot).pace.swing
    if strike == nil or strike.swings == 0 or swing == nil then
        return nil
    end
    local swings = math.ceil(mob.hp / (strike.hp / strike.swings))
    local hits   = math.ceil(swings * swing / math.max(mob.speed, 1))
    return hits * mob.damage
end

function M.winnable(brain, bot, mob)
    local hp   = brain:observe(bot).hp
    local cost = M.cost(brain, bot, mob)
    if cost == nil then
        return mob.damage < hp
    else
        return cost < hp
    end
end

function M.guard(mob)
    return M.hits(mob) and mob.damage or 0
end

function M.threat(brain, bot)
    local threat = 0
    for _, mob in ipairs(brain:mobs(bot)) do
        if mob.damage ~= nil and M.hits(mob) then
            threat = math.max(threat, mob.damage)
        end
    end
    return threat
end

function M.fatal(brain, bot)
    for _, mob in ipairs(brain:mobs(bot)) do
        if mob.attack == "AGGRESSIVE" and M.winnable(brain, bot, mob) == false then
            return mob
        end
    end
    return nil
end

return M
