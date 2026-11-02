local protocol = require("integration.protocol")
local resp     = require("integration.response")
local wait     = require("brain.policies.wait")

local M = {}

local EMPTY_SLOT  = 0xFF
local DIALOG_WAIT = 3000

function M.consume(bot, name)
    local slot = bot:item_slot(name)
    if slot == EMPTY_SLOT then
        return false
    end
    local response = bot:item_count(name) > 1 and resp.item_update or resp.item_remove
    return wait.reply(bot, response, protocol.item_active(slot), function(packet)
        return packet.index == slot
    end, DIALOG_WAIT) ~= nil
end

return M
