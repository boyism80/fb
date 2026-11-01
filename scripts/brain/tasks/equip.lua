local protocol = require("integration.protocol")
local resp     = require("integration.response")
local gear     = require("brain.data.gear")
local wait     = require("brain.policies.wait")

local M = {}

local WEAR_WAIT = 3000

local function score(brain, name)
    local item = brain:item(name)
    if item == nil then
        return nil
    end
    local weights = gear.weights[item.type]
    if weights == nil then
        return nil
    end

    local total = 0
    for stat, weight in pairs(weights) do
        total = total + (item[stat] or 0) * weight
    end
    return total
end

local function worn(brain, bot)
    local result = {}
    for _, slot in ipairs(brain:worn(bot)) do
        local lowest = nil
        for _, name in ipairs(slot.names) do
            local value = score(brain, name)
            if value ~= nil and (lowest == nil or value < lowest) then
                lowest = value
            end
        end
        if result[slot.type] == nil or (lowest or -math.huge) < result[slot.type] then
            result[slot.type] = lowest or -math.huge
        end
    end
    return result
end

function M.best(brain, bot)
    local current = worn(brain, bot)
    local best    = nil
    for _, item in ipairs(brain:inventory(bot)) do
        local value = score(brain, item.name)
        local base  = current[item.type]
        if value ~= nil and base ~= nil and value > base and (best == nil or value > best.value) then
            best = { slot = item.slot, name = item.name, value = value }
        end
    end
    return best
end

function M.wanted(brain, bot, seen)
    return seen.ghost == false and seen.class ~= "NONE" and M.best(brain, bot) ~= nil
end

function M.run(brain, bot)
    local best = M.best(brain, bot)
    if best == nil then
        return true
    end
    local worn = wait.reply(bot, resp.message, protocol.item_active(best.slot), function(packet)
        return packet.type == "STATE"
    end, WEAR_WAIT)
    if worn == false or worn == nil then
        return false, "REFUSED"
    end
    wait.reply(bot, resp.internal_info, protocol.self_info(), function() return true end, WEAR_WAIT)
    if M.best(brain, bot) ~= nil and M.best(brain, bot).name == best.name then
        return false, "REFUSED"
    end
    return true
end

return M
