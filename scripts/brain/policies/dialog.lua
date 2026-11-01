local protocol = require("integration.protocol")
local resp     = require("integration.response")
local wait     = require("brain.policies.wait")

local M = {}

local DIALOG_CLOSE = 0xFFFFFFFD
local EMPTY_SLOT   = 0xFF
local DIALOG_WAIT  = 3000
local ANSWERS      = { list = "LIST", menu = "MENU", pursuit = "PURSUIT" }

function M.settled(bot)
    return wait.reply(bot, resp.internal_info, protocol.self_info(), function() return true end, DIALOG_WAIT) ~= nil
end

local function answer(bot, packet)
    local seq = bot:last_dialog()
    bot:send(packet)
    if M.settled(bot) == false then
        return nil
    end
    local now, dialog = bot:last_dialog()
    if now == seq then
        return nil
    end
    return dialog
end

local function open(brain, bot, packet)
    if brain:ask(bot, packet) == false then
        return nil
    end
    local _, dialog = bot:last_dialog()
    return dialog
end

function M.converse(brain, bot, opening, answers)
    if brain:active(bot) == false then
        return
    end
    local index  = 1
    local dialog = open(brain, bot, opening)
    while dialog ~= nil and brain:active(bot) do
        local kind = ANSWERS[dialog.type]
        if kind ~= nil and answers[index] ~= nil then
            local choice = answers[index]
            local packet = type(choice) == "string" and protocol.dialog(kind, 0, "", 0, 0, choice, "NEXT")
                or protocol.dialog(kind, 0, "", choice, 0, "", "NEXT")
            dialog       = open(brain, bot, packet)
            index        = index + 1
        elseif dialog.normal_button_next then
            dialog = open(brain, bot, protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"))
        else
            answer(bot, protocol.dialog("NORMAL", 0, "", 0, 0, "", "QUIT"))
            dialog = nil
        end
    end
    bot:send(protocol.click(DIALOG_CLOSE))
end

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
