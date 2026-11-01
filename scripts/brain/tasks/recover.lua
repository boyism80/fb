local protocol = require("integration.protocol")
local resp     = require("integration.response")
local travel   = require("brain.policies.travel")
local wait     = require("brain.policies.wait")

local M = {}

local WEARABLE  = { WEAPON = true, ARMOR = true, HELMET = true, RING = true, SHIELD = true, AUXILIARY = true }
local WEAR_WAIT = 3000

local function holding(brain, bot)
    local counts = {}
    for _, item in ipairs(brain:inventory(bot)) do
        counts[item.name] = (counts[item.name] or 0) + item.count
    end
    return counts
end

local function pile_at(brain, bot, tried)
    local here = bot:position()
    for _, item in ipairs(brain:items(bot)) do
        if item.x == here[1] and item.y == here[2] and tried[item.oid] == nil then
            return item
        end
    end
    return nil
end

function M.missing(brain, bot, death)
    local counts = holding(brain, bot)
    if bot:money() < death.money then
        return true
    end
    for name, count in pairs(death.items) do
        if (counts[name] or 0) < count then
            return true
        end
    end
    return false
end

local function wear(brain, bot, death)
    local counts = holding(brain, bot)
    for _, item in ipairs(brain:inventory(bot)) do
        if WEARABLE[item.type] and counts[item.name] > (death.items[item.name] or 0) then
            wait.reply(bot, resp.message, protocol.item_active(item.slot), function(packet)
                return packet.type == "STATE"
            end, WEAR_WAIT)
            counts[item.name] = counts[item.name] - 1
        end
    end
end

function M.wanted(brain, bot, seen)
    return seen.ghost == false and brain:death(bot) ~= nil
end

function M.run(brain, bot)
    local death = brain:death(bot)
    if death == nil then
        return true
    end

    wear(brain, bot, death)
    if death.left <= 0 or M.missing(brain, bot, death) == false then
        brain:bury(bot)
        return true
    end

    local arrived, event = travel.go(brain, bot, { map = death.map, x = death.x, y = death.y, range = 0 })
    if arrived == false and event ~= nil and event.type == "NO_ROUTE" then
        brain:bury(bot)
        return true
    elseif arrived == false then
        return false, event and event.type
    end
    local tried = {}
    local pile  = pile_at(brain, bot, tried)
    while brain:active(bot) and pile ~= nil do
        tried[pile.oid] = true
        brain:loot(bot)
        pile = pile_at(brain, bot, tried)
    end
    wear(brain, bot, death)
    brain:bury(bot)
    return true
end

return M
