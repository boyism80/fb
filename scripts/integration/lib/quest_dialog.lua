-- Dialog and state-check helpers for NPC quest tests.
-- One NPC at a time is spawned below the bot and swapped on demand.
local protocol    = require("integration.protocol")
local resp        = require("integration.response")
local script_step = require("integration.lib.script_step")

local M = {}

local DIALOG_CLOSE_OID = 0xFFFFFFFD

-- Spawned NPC per bot name: { name = string, oid = number }.
local g_npcs = {}

function M.contains(text)
    return function(p)
        return p.message ~= nil and p.message:find(text, 1, true) ~= nil
    end
end

function M.is_list(p)
    return p.type == "list"
end

-- A dialog button resumes the NPC script inside the server's dialog handler, so its state change
-- is applied before a check sent afterwards on the same session runs.
function M.check(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

function M.remove_npc(bot)
    local npc = g_npcs[bot:name()]
    if npc == nil then
        return
    end
    bot:direction("BOTTOM")
    bot:request(resp.hide, protocol.chat(false, "/엔피씨제거"), function(packet)
        return packet.oid == npc.oid
    end)
    g_npcs[bot:name()] = nil
end

function M.use_npc(bot, name)
    local current = g_npcs[bot:name()]
    if current ~= nil and current.name == name then
        return current.oid
    end
    M.remove_npc(bot)
    bot:direction("BOTTOM")
    bot:move("BOTTOM")
    local oid = bot:create_npc(name).oid
    bot:move("TOP")
    g_npcs[bot:name()] = { name = name, oid = oid }
    return oid
end

function M.close(bot)
    bot:send(protocol.click(DIALOG_CLOSE_OID))
end

function M.click(bot, name, expect)
    local oid = M.use_npc(bot, name)
    return bot:request_dialog_ext(protocol.click(oid), expect)
end

-- NPCs that open with a me:pursuit menu.
function M.open_pursuit(bot, name, option, expect)
    local oid = M.use_npc(bot, name)
    if bot:request_dialog(protocol.click(oid), function(p) return p.type == "pursuit" end) == nil then
        return nil
    end
    return bot:request_dialog_ext(protocol.dialog("PURSUIT", 0, "", 0, 0, option), expect)
end

function M.next_dialog(bot, expect)
    return bot:request_dialog_ext(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"), expect)
end

function M.prev_dialog(bot, expect)
    return bot:request_dialog_ext(protocol.dialog("NORMAL", 0, "", 0, 0, "", "PREV"), expect)
end

-- A final me:dialog or a dialog after which the script only changes state has no reply.
function M.press(bot, button)
    bot:send(protocol.dialog("NORMAL", 0, "", 0, 0, "", button))
end

local function option_index(list_packet, option)
    for i, text in ipairs(list_packet.list_lists or {}) do
        if text == option then
            return i
        end
    end
    return nil
end

function M.select_option(bot, list_packet, option, expect)
    local index = option_index(list_packet, option)
    if index == nil then
        return nil
    end
    return bot:request_dialog_ext(protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"), expect)
end

-- Selecting an option after which the script ends without another dialog.
function M.choose(bot, list_packet, option)
    local index = option_index(list_packet, option)
    if index == nil then
        return false
    end
    bot:send(protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"))
    return true
end

return M
