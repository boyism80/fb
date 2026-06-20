local protocol = require("integration.protocol")

local M = {}

local function normalize_dialog(packet)
    if packet == nil then
        return nil
    end
    return {
        type        = packet.type,
        dialog_oid  = packet.dialog_oid,
        dialog_name = packet.dialog_name,
    }
end

local function normalize_bundle(packet)
    if packet == nil then
        return nil
    end
    return { type = packet.type }
end

local function normalize_upload(packet)
    if packet == nil then
        return nil
    end
    return {
        type         = packet.type,
        upload_index = packet.upload_index,
        upload_look  = packet.upload_look,
        upload_color = packet.upload_color,
        upload_name  = packet.upload_name,
        upload_mine  = packet.upload_mine,
    }
end

local function normalize_money(packet)
    if packet == nil then
        return nil
    end
    return {
        type  = packet.type,
        money = packet.money,
        mine  = packet.mine,
    }
end

local function normalize_close(packet)
    if packet == nil then
        return nil
    end
    return {
        type          = packet.type,
        close_message = packet.close_message,
    }
end

local function normalize_lock(packet)
    if packet == nil then
        return nil
    end
    return {
        type          = packet.type,
        close_message = packet.close_message,
    }
end

local function run_trade_request(bot, request_packet, normalize, validator)
    log("debug", string.format("trade: request_trade call bot_id=%s", tostring(bot:oid())))

    local packet = bot:request_trade(
        request_packet,
        function(raw)
            if raw == nil or raw == false then
                log("debug", "trade: request_trade validator raw=nil")
                return false
            end

            local unified = normalize(raw)
            if validator ~= nil then
                return validator(unified)
            end
            return true
        end)

    if packet == nil or packet == false then
        log("debug", "trade: request_trade returned nil")
        return nil
    end

    log("debug", string.format("trade: request_trade ok type=%s", tostring(packet.type)))
    return normalize(packet)
end

function M.slot(index)
    return index + 1
end

function M.type_is(trade_type)
    return function(packet)
        return packet.type == trade_type
    end
end

function M.dialog_oid(target_oid)
    return function(packet)
        return packet.type == "dialog" and packet.dialog_oid == target_oid
    end
end

function M.close_contains(text)
    return function(packet)
        if packet.type ~= "close" then
            return false
        end
        if packet.close_message == nil then
            return false
        end
        return string.find(packet.close_message, text, 1, true) ~= nil
    end
end

function M.request(bot, target_oid, validator)
    log("debug", string.format("trade: building request packet target_oid=%s", tostring(target_oid)))
    local request_packet = protocol.trade("REQUEST", target_oid, {})
    log("debug", "trade: request packet built")
    return run_trade_request(bot, request_packet, normalize_dialog, validator)
end

function M.up_item(bot, target_oid, slot_index, validator)
    return run_trade_request(
        bot,
        protocol.trade("UP_ITEM", target_oid, { index = M.slot(slot_index) }),
        normalize_bundle,
        validator or M.type_is("bundle"))
end

function M.item_count(bot, target_oid, count, validator)
    return run_trade_request(
        bot,
        protocol.trade("ITEM_COUNT", target_oid, { count = count }),
        normalize_upload,
        validator or M.type_is("upload"))
end

function M.up_item_upload(bot, target_oid, slot_index, validator)
    return run_trade_request(
        bot,
        protocol.trade("UP_ITEM", target_oid, { index = M.slot(slot_index) }),
        normalize_upload,
        validator or M.type_is("upload"))
end

function M.up_money(bot, target_oid, money, validator)
    return run_trade_request(
        bot,
        protocol.trade("UP_MONEY", target_oid, { money = money }),
        normalize_money,
        validator or M.type_is("money"))
end

function M.cancel(bot, target_oid, validator)
    return run_trade_request(
        bot,
        protocol.trade("CANCEL", target_oid, {}),
        normalize_close,
        validator)
end

function M.lock(bot, target_oid, validator)
    return run_trade_request(
        bot,
        protocol.trade("LOCK", target_oid, {}),
        normalize_lock,
        validator or M.type_is("lock"))
end

function M.put_full_offer(bot, target_oid, acorn_count, money, extra_slot_index)
    M.up_item(bot, target_oid, 0)
    M.item_count(bot, target_oid, acorn_count)
    M.up_money(bot, target_oid, money)
    if extra_slot_index ~= nil then
        M.up_item_upload(bot, target_oid, extra_slot_index)
    end
end

return M
