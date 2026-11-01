local protocol = require("integration.protocol")
local dialog   = require("brain.policies.dialog")
local travel   = require("brain.policies.travel")

local M = {}

local REVIVERS = { "좌성황", "우성황", "공성성황", "성황할머니" }
local YES      = 1
local OATH     = { YES, YES, YES, YES }

function M.wanted(brain, bot, seen)
    return seen.ghost
end

function M.run(brain, bot)
    local spots = brain:npcs(bot, REVIVERS)
    local maps  = {}
    for _, spot in ipairs(spots) do
        table.insert(maps, spot.map)
    end
    local arrived, event = travel.go(brain, bot, { maps = maps })
    if arrived == false then
        return false, event and event.type
    end

    local spot = nil
    for _, candidate in ipairs(spots) do
        if candidate.map == bot:map() then
            spot = candidate
        end
    end
    if spot == nil or travel.go(brain, bot, { map = spot.map, x = spot.x, y = spot.y, range = 1 }) == false then
        return false
    end

    local npc = brain:npc(bot, spot.name)
    if npc == nil then
        return false
    end
    dialog.converse(brain, bot, protocol.click(npc.oid), OATH)
    return brain:observe(bot).ghost == false
end

return M
