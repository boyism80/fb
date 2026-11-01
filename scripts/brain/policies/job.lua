local protocol = require("integration.protocol")
local resp     = require("integration.response")
local job      = require("brain.data.job")
local wait     = require("brain.policies.wait")

local M = {}

local USERS_WAIT = 3000

function M.shares()
    local members = job.party.attackers + job.party.healers
    local shares  = {}
    for _, group in ipairs({ { job.attackers, job.party.attackers }, { job.healers, job.party.healers } }) do
        local roles, seats = group[1], group[2]
        local total = 0
        for _, weight in pairs(roles) do
            total = total + weight
        end
        for name, weight in pairs(roles) do
            shares[name] = seats / members * weight / total
        end
    end
    return shares
end

function M.deficits(population)
    local shares = M.shares()
    local total  = 1
    for name in pairs(shares) do
        total = total + (population[name] or 0)
    end

    local deficits = {}
    local any      = false
    for name, share in pairs(shares) do
        deficits[name] = math.max(0, share - (population[name] or 0) / total)
        any = any or deficits[name] > 0
    end
    return any and deficits or shares
end

local function census(brain, bot)
    local population = brain:recall(bot, "population")
    if population == nil then
        population = {}
        local packet = wait.reply(bot, resp.user_list, protocol.user_list(), function() return true end, USERS_WAIT)
        for _, user in ipairs(packet and packet.users or {}) do
            local name = job.codes[user.cls]
            if name ~= nil and user.name ~= bot:name() then
                population[name] = (population[name] or 0) + 1
            end
        end
    end

    local counts = {}
    for name, count in pairs(population) do
        counts[name] = count
    end
    for name, count in pairs(brain:reservations(bot)) do
        counts[name] = (counts[name] or 0) + count
    end
    return counts
end

function M.choose(brain, bot)
    local chosen = brain:recall(bot, "job")
    if chosen == nil then
        chosen = brain:pick(bot, M.deficits(census(brain, bot)))
        if chosen == nil then
            return nil
        end
        brain:remember(bot, "job", chosen)
    end
    brain:reserve(bot, chosen)
    return chosen
end

function M.settle(brain, bot)
    brain:release(bot)
    brain:remember(bot, "job", nil)
end

return M
