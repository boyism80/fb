local lib      = require("integration.lib")
local protocol = require("integration.protocol")

local F1_OID               = 0xFFFFFFFF
local F1_STORAGE_INDEX     = 1
local F1_MARKETPLACE_INDEX = 2
local MP_SEARCH_INDEX      = 1
local MP_LIST_INDEX        = 2
local MP_CANCEL_INDEX      = 3
local CONFIRM_YES          = 1
local CONFIRM_NO           = 2
local RECEIVE_YES          = 1

local DIALOG_PREV = 0
local DIALOG_NEXT = 2

local WEAPON_ITEM       = "양첨목봉"
local BUNDLE_ITEM       = "도토리"
local WEAPON_LIST_PRICE = 10000
local BUNDLE_UNIT_PRICE = 100
local BUNDLE_LIST_COUNT = 100
local BUNDLE_PREP_COUNT = 200
local LISTING_FEE_RATE  = 0.05
local WEAPON_DURABILITY = 50

local MQ_WAIT_MS     = 2000
local HTTP_DELAY_MS  = 3000
local DELAY_PROBE_MS = 3500
local DIALOG_TIMEOUT_MS = 30000

local MSG_FEE_FAIL       = "등록 수수료가 부족합니다"
local MSG_LIST_OK        = "등록이 완료되었습니다"
local MSG_LIST_FAIL      = "등록 실패"
local MSG_PURCHASE_OK    = "구매가 완료되었습니다"
local MSG_CANCEL_OK      = "취소가 완료되었습니다"
local MSG_FEE_CONFIRM    = "등록하시겠습니까"
local MSG_CANCEL_CONFIRM = "정말 취소하시겠습니까"
local MSG_WARNING        = "정말 구매하시겠습니까"
local MSG_SEARCH_EMPTY = "검색 결과가 없습니다"
local MSG_CANCEL_EMPTY = "등록한 물품이 없습니다"

local STORAGE_CANCEL_TITLE   = "거래소 등록 취소"
local STORAGE_PURCHASE_TITLE = "거래소 구매"
local STORAGE_SALE_TITLE     = "거래소 판매"

-- Shared across sequential/parallel scenario steps
local g_list_msg     = nil
local g_purchase_msg = nil
local g_cancel_msg   = nil
local g_search_ok    = false
local g_search_err   = nil
local g_money_ok              = false
local g_pay_amount            = 0
local g_purchase_api_started  = false

local function listing_fee(count, price)
    return math.floor(count * price * LISTING_FEE_RATE)
end

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("marketplace_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function dump_dialog(bot, label, packet)
    if packet == nil then
        progress(bot, string.format("DIALOG[%s] nil", label))
        return
    end
    local lists = ""
    if packet.list_lists ~= nil then
        lists = table.concat(packet.list_lists, "|")
    end
    local slots = ""
    if packet.slot_slots ~= nil then
        local parts = {}
        for i, s in ipairs(packet.slot_slots) do
            parts[i] = tostring(s)
        end
        slots = table.concat(parts, ",")
    end
    local items = ""
    if packet.item_items ~= nil then
        local parts = {}
        for i, it in ipairs(packet.item_items) do
            parts[i] = tostring(it.name)
        end
        items = table.concat(parts, ",")
    end
    progress(bot, string.format(
        "DIALOG[%s] type=%s msg=%q lists=%q slots=[%s] items=[%s]",
        label,
        tostring(packet.type),
        tostring(packet.message),
        lists,
        slots,
        items))
end

local function cleanup_bot(bot)
    bot:chat("/HTTP지연 0")
    bot:chat("/아이템초기화")
    bot:chat("/아이템삭제")
    bot:money(0)
end

local function message_contains(packet, text)
    return packet ~= nil
        and packet.message ~= nil
        and packet.message:find(text, 1, true) ~= nil
end

-- Server messages may contain irregular spacing (e.g. "등록한 물품이  없습니다.").
local function message_normalized_contains(packet, text)
    if packet == nil or packet.message == nil then
        return false
    end
    local normalized = packet.message:gsub("%s+", " ")
    return normalized:find(text, 1, true) ~= nil
end

local function is_cancel_empty_message(packet)
    return message_normalized_contains(packet, MSG_CANCEL_EMPTY)
end

local function is_search_empty_message(packet)
    return message_normalized_contains(packet, MSG_SEARCH_EMPTY)
end

local function dialog_normal_next()
    return protocol.dialog("NORMAL", DIALOG_NEXT, "", 0, 0, "", "NEXT")
end

local function f1_open_menu(bot)
    progress(bot, "f1_open_menu")
    local packet = bot:request_dialog_ext(
        protocol.click(F1_OID),
        function(p)
            return p.type == "list"
        end)
    dump_dialog(bot, "f1_open", packet)
    return packet
end

local function f1_select_list(bot, index)
    progress(bot, string.format("f1_select_list index=%d", index))
    local packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "f1_select_list", packet)
    return packet
end

local function f1_select_input(bot, index)
    progress(bot, string.format("f1_select_input index=%d", index))
    local packet = bot:request_dialog(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "input"
        end)
    dump_dialog(bot, "f1_select_input", packet)
    return packet
end

local function f1_select_slot(bot, index)
    progress(bot, string.format("f1_select_slot index=%d", index))
    local packet = bot:request_dialog(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "slot" or p.type == "normal"
        end)
    dump_dialog(bot, "f1_select_slot", packet)
    return packet
end

local function dismiss_normal_to_list(bot)
    progress(bot, "dismiss_normal_to_list")
    local packet = bot:request_dialog_ext(
        dialog_normal_next(),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "dismiss_normal", packet)
    return packet
end

local function close_marketplace_menu(bot)
    progress(bot, "close_marketplace_menu PREV")
    local packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", 0, 0, "", "PREV"),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "close_mp_menu", packet)
    return packet
end

local function open_marketplace_menu(bot)
    local packet = f1_open_menu(bot)
    if packet == nil then
        return nil, "f1 open failed"
    end
    packet = f1_select_list(bot, F1_MARKETPLACE_INDEX)
    if packet == nil or packet.type ~= "list" then
        return nil, "marketplace menu missing"
    end
    return packet, nil
end

local function find_list_index(packet, text)
    if packet == nil or packet.list_lists == nil then
        return nil
    end
    for i, name in ipairs(packet.list_lists) do
        if name == text then
            return i
        end
    end
    return nil
end

local function resolve_list_slot(bot, item_name, slot_packet)
    local invent_0 = bot:item_slot(item_name)
    if invent_0 == nil or invent_0 == 0xFF then
        invent_0 = nil
    end

    local dialog_slots_1based = {}
    if slot_packet ~= nil and slot_packet.slot_slots ~= nil then
        for _, s in ipairs(slot_packet.slot_slots) do
            table.insert(dialog_slots_1based, s)
        end
    end

    progress(bot, string.format(
        "resolve_list_slot item=%s invent_0=%s dialog_slots_1based=[%s]",
        item_name,
        tostring(invent_0),
        table.concat(dialog_slots_1based, ",")))

    if invent_0 ~= nil then
        local invent_1 = invent_0 + 1
        for _, s in ipairs(dialog_slots_1based) do
            if s == invent_1 then
                return invent_1
            end
        end
    end

    if #dialog_slots_1based > 0 then
        local slot_1 = dialog_slots_1based[1]
        progress(bot, string.format("resolve_list_slot fallback slot_1=%s", tostring(slot_1)))
        return slot_1
    end

    if invent_0 ~= nil then
        return invent_0 + 1
    end

    return nil
end

local function open_storage_entry_detail(bot, title)
    local packet = f1_open_menu(bot)
    if packet == nil then
        return nil, "f1 open failed"
    end

    packet = f1_select_list(bot, F1_STORAGE_INDEX)
    if packet == nil then
        return nil, "storage list missing"
    end

    if packet.type == "normal" and message_contains(packet, "보관된 항목이 없습니다") then
        dismiss_normal_to_list(bot)
        return nil, "storage empty"
    end

    local index = find_list_index(packet, title)
    if index == nil then
        close_marketplace_menu(bot)
        return nil, "entry not found: " .. title
    end

    packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return nil, "entry detail missing"
    end
    return packet, nil
end

local function receive_storage(bot, title)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail == nil then
        return false, err
    end

    local packet = bot:request_dialog_ext(
        dialog_normal_next(),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    if packet == nil or packet.type ~= "list" then
        return false, "receive confirm missing"
    end

    packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", RECEIVE_YES, 0, "", "NEXT"),
        function(p)
            return p.type == "normal" and p.message ~= nil
        end)
    if packet == nil then
        return false, "receive result missing"
    end

    dismiss_normal_to_list(bot)
    return true, packet.message
end

local function prepare_weapon(bot)
    bot:create_item(WEAPON_ITEM, 1)
    bot:chat(string.format("/내구도 %d", WEAPON_DURABILITY))
    return bot:has_item_by_name(WEAPON_ITEM)
end

local function prepare_bundle(bot)
    bot:create_item(BUNDLE_ITEM, BUNDLE_PREP_COUNT)
    return bot:item_count(BUNDLE_ITEM) == BUNDLE_PREP_COUNT
end

local function list_item_flow(bot, item_name, count, price)
    progress(bot, string.format(
        "list_item_flow start item=%s count=%s price=%s money=%s invent=%s",
        item_name,
        tostring(count),
        tostring(price),
        tostring(bot:money()),
        tostring(bot:has_item_by_name(item_name))))

    local packet, err = open_marketplace_menu(bot)
    if packet == nil then
        return nil, err
    end

    packet = f1_select_slot(bot, MP_LIST_INDEX)
    if packet == nil then
        return nil, "list slot dialog missing"
    end
    if packet.type == "normal" then
        local msg = packet.message
        progress(bot, "list_item_flow early normal: " .. tostring(msg))
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return msg, nil
    end

    local slot = resolve_list_slot(bot, item_name, packet)
    if slot == nil then
        return nil, "item slot not found: " .. item_name
    end

    progress(bot, string.format("list_item_flow send SLOT index=%d", slot))
    if count ~= nil and count > 1 then
        packet = bot:request_dialog(
            protocol.dialog("SLOT", 0, "", slot, 0, "", "NEXT"),
            function(p)
                return p.type == "input" or p.type == "slot" or p.type == "normal"
            end)
        dump_dialog(bot, "after_slot_count", packet)
        if packet == nil then
            return nil, "count input missing"
        end
        if packet.type ~= "input" then
            return nil, "expected count input, got=" .. tostring(packet.type) .. " msg=" .. tostring(packet.message)
        end

        progress(bot, string.format("list_item_flow send count=%d", count))
        packet = bot:request_dialog(
            protocol.dialog("INPUT", 0, tostring(count), 0, 0, "", "NEXT"),
            function(p)
                return p.type == "input" or p.type == "normal"
            end)
        dump_dialog(bot, "after_count_input", packet)
    else
        packet = bot:request_dialog(
            protocol.dialog("SLOT", 0, "", slot, 0, "", "NEXT"),
            function(p)
                return p.type == "input" or p.type == "slot" or p.type == "normal"
            end)
        dump_dialog(bot, "after_slot_price", packet)
    end
    if packet == nil then
        return nil, "price input missing"
    end
    if packet.type ~= "input" then
        return nil, "expected price input, got=" .. tostring(packet.type) .. " msg=" .. tostring(packet.message)
    end

    progress(bot, string.format("list_item_flow send price=%d", price))
    packet = bot:request_dialog_ext(
        protocol.dialog("INPUT", 0, tostring(price), 0, 0, "", "NEXT"),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "after_price_input", packet)
    if packet == nil then
        return nil, "fee confirm missing"
    end

    if packet.type == "normal" then
        local msg = packet.message
        progress(bot, "list_item_flow result normal: " .. tostring(msg))
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return msg, nil
    end

    if message_contains(packet, MSG_FEE_CONFIRM) == false then
        return nil, "unexpected fee dialog: " .. tostring(packet.message)
    end

    progress(bot, "list_item_flow confirm YES")
    packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", CONFIRM_YES, 0, "", "NEXT"),
        function(p)
            return p.type == "normal" and p.message ~= nil
        end)
    dump_dialog(bot, "list_result", packet)
    if packet == nil then
        return nil, "list result missing"
    end

    local msg = packet.message
    dismiss_normal_to_list(bot)
    close_marketplace_menu(bot)
    progress(bot, "list_item_flow done msg=" .. tostring(msg))
    return msg, nil
end

local function abort_search_item_dialog(bot, item_name)
    progress(bot, "abort_search_item_dialog item=" .. item_name)
    local after = bot:request_dialog_ext(
        protocol.dialog("ITEM", 0, "", 0, 0, item_name, "NEXT"),
        function(p)
            return p.type == "input" or p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "abort_after_item", after)
    if after == nil then
        close_marketplace_menu(bot)
        return
    end

    if after.type == "list" and message_contains(after, MSG_WARNING) then
        after = bot:request_dialog(
            protocol.dialog("LIST", 0, "", CONFIRM_NO, 0, "", "NEXT"),
            function(p)
                return p.type == "input" or p.type == "list" or p.type == "normal"
            end)
        dump_dialog(bot, "abort_after_no", after)
    end

    if after ~= nil and after.type == "input" then
        local next_packet = bot:request_dialog_ext(
            protocol.dialog("INPUT", 0, "", 0, 0, "", "NEXT"),
            function(p)
                return p.type == "list" or p.type == "normal"
            end)
        dump_dialog(bot, "abort_after_empty_input", next_packet)
        if next_packet ~= nil and next_packet.type == "normal" then
            dismiss_normal_to_list(bot)
        end
    elseif after ~= nil and after.type == "normal" then
        dismiss_normal_to_list(bot)
    end

    close_marketplace_menu(bot)
end

local function search_listings_expect_empty(bot, item_name)
    progress(bot, string.format("search_listings_expect_empty item=%s", item_name))
    local packet, err = open_marketplace_menu(bot)
    if packet == nil then
        return nil, err
    end

    packet = f1_select_input(bot, MP_SEARCH_INDEX)
    if packet == nil then
        return nil, "search input missing"
    end

    local input_req = protocol.dialog("INPUT", 0, item_name, 0, 0, "", "NEXT")

    packet = bot:request_dialog_ext(
        input_req,
        function(p)
            return p.type == "normal"
        end,
        DIALOG_TIMEOUT_MS)
    dump_dialog(bot, "search_result_empty", packet)
    if packet == nil then
        return nil, "search empty result missing"
    end
    return packet, nil
end

local function search_listings_expect_present(bot, item_name)
    progress(bot, string.format("search_listings_expect_present item=%s", item_name))
    local packet, err = open_marketplace_menu(bot)
    if packet == nil then
        return nil, err
    end

    packet = f1_select_input(bot, MP_SEARCH_INDEX)
    if packet == nil then
        return nil, "search input missing"
    end

    packet = bot:request_dialog(
        protocol.dialog("INPUT", 0, item_name, 0, 0, "", "NEXT"),
        function(p)
            return p.type == "item" or p.type == "normal"
        end,
        DIALOG_TIMEOUT_MS)
    dump_dialog(bot, "search_result", packet)
    if packet == nil then
        return nil, "search result missing"
    end
    return packet, nil
end

local function cancel_menu_request()
    return protocol.dialog("LIST", 0, "", MP_CANCEL_INDEX, 0, "", "NEXT")
end

local function open_cancel_list_expect_present(bot)
    local packet, err = open_marketplace_menu(bot)
    if packet == nil then
        return nil, err
    end

    packet = bot:request_dialog(
        cancel_menu_request(),
        function(p)
            return p.type == "item"
        end,
        DIALOG_TIMEOUT_MS)
    dump_dialog(bot, "cancel_list", packet)
    if packet == nil then
        return nil, "cancel list missing"
    end
    return packet, nil
end

local function open_cancel_list_expect_empty(bot)
    local packet, err = open_marketplace_menu(bot)
    if packet == nil then
        return nil, err
    end

    packet = bot:request_dialog_ext(
        cancel_menu_request(),
        function(p)
            return p.type == "normal"
        end,
        DIALOG_TIMEOUT_MS)
    dump_dialog(bot, "cancel_list_empty", packet)
    if packet == nil then
        return nil, "cancel list empty missing"
    end
    return packet, nil
end

local function assert_search_has_item(bot, item_name, expect_found)
    progress(bot, string.format("assert_search_has_item item=%s expect_found=%s", item_name, tostring(expect_found)))
    if expect_found then
        local packet, err = search_listings_expect_present(bot, item_name)
        if packet == nil then
            return false, tostring(err)
        end
        if packet.type ~= "item" then
            local msg = packet.message
            dismiss_normal_to_list(bot)
            close_marketplace_menu(bot)
            return false, "expected listings, got=" .. tostring(msg)
        end
        abort_search_item_dialog(bot, item_name)
        return true, nil
    end

    local packet, err = search_listings_expect_empty(bot, item_name)
    if packet == nil then
        return false, tostring(err)
    end

    if packet.type == "item" then
        abort_search_item_dialog(bot, item_name)
        return false, "listing unexpectedly found"
    end

    if message_contains(packet, MSG_SEARCH_EMPTY) == false and is_search_empty_message(packet) == false then
        local msg = packet.message
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return false, "expected empty search, got=" .. tostring(msg)
    end

    dismiss_normal_to_list(bot)
    close_marketplace_menu(bot)
    return true, nil
end

local function count_search_items(packet, item_name)
    local n = 0
    if packet == nil or packet.item_items == nil then
        return n
    end
    for _, it in ipairs(packet.item_items) do
        if it.name == item_name then
            n = n + 1
        end
    end
    return n
end

local function purchase_selected_item(bot, item_name, purchase_count, listing_count)
    listing_count = listing_count or 1
    local need_count_input = purchase_count ~= nil and purchase_count > 1
    progress(bot, string.format(
        "purchase_selected_item item=%s count=%s listings=%d money=%s",
        item_name,
        tostring(purchase_count),
        listing_count,
        tostring(bot:money())))

    local packet
    if listing_count > 1 then
        -- Same-name multi listing -> LIST (0x30), then count INPUT (0x2F) or warning/result.
        packet = bot:request_dialog_ext(
            protocol.dialog("ITEM", 0, "", 0, 0, item_name, "NEXT"),
            function(p)
                return p.type == "list" or p.type == "normal"
            end)
        dump_dialog(bot, "purchase_multi_list", packet)
        if packet == nil then
            return nil, "multi listing dialog missing"
        end
        if packet.type == "list" then
            if need_count_input then
                packet = bot:request_dialog(
                    protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"),
                    function(p)
                        return p.type == "input" or p.type == "normal"
                    end)
            else
                packet = bot:request_dialog_ext(
                    protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"),
                    function(p)
                        return p.type == "list" or p.type == "normal"
                    end)
            end
            dump_dialog(bot, "purchase_after_multi", packet)
        end
    elseif need_count_input then
        packet = bot:request_dialog(
            protocol.dialog("ITEM", 0, "", 0, 0, item_name, "NEXT"),
            function(p)
                return p.type == "input" or p.type == "normal"
            end)
        dump_dialog(bot, "purchase_after_item", packet)
    else
        packet = bot:request_dialog_ext(
            protocol.dialog("ITEM", 0, "", 0, 0, item_name, "NEXT"),
            function(p)
                return p.type == "list" or p.type == "normal"
            end)
        dump_dialog(bot, "purchase_after_item", packet)
    end
    if packet == nil then
        return nil, "after item select missing"
    end

    if packet.type == "input" then
        progress(bot, "purchase send count=" .. tostring(purchase_count))
        packet = bot:request_dialog_ext(
            protocol.dialog("INPUT", 0, tostring(purchase_count), 0, 0, "", "NEXT"),
            function(p)
                return p.type == "list" or p.type == "normal"
            end)
        dump_dialog(bot, "purchase_after_count", packet)
        if packet == nil then
            return nil, "after count input missing"
        end
    end

    if packet.type == "list" then
        progress(bot, string.format(
            "purchase confirm YES money_cache=%d",
            bot:money()))
        g_purchase_api_started = true
        packet = bot:request_dialog_ext(
            protocol.dialog("LIST", 0, "", CONFIRM_YES, 0, "", "NEXT"),
            function(p)
                return p.type == "normal" and p.message ~= nil
            end)
        dump_dialog(bot, "purchase_result", packet)
    end

    if packet == nil or packet.type ~= "normal" then
        return nil, "purchase result missing type=" .. tostring(packet and packet.type)
    end

    local msg = packet.message
    dismiss_normal_to_list(bot)
    close_marketplace_menu(bot)
    progress(bot, "purchase_selected_item done msg=" .. tostring(msg))
    return msg, nil
end

local function purchase_item_flow(bot, item_name, purchase_count)
    progress(bot, string.format("purchase_item_flow item=%s count=%s", item_name, tostring(purchase_count)))
    local packet, err = search_listings_expect_present(bot, item_name)
    if packet == nil then
        return nil, err
    end
    if packet.type ~= "item" then
        local msg = packet.message
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return nil, msg
    end
    local listing_count = count_search_items(packet, item_name)
    if listing_count < 1 then
        listing_count = 1
    end
    return purchase_selected_item(bot, item_name, purchase_count, listing_count)
end

local function cancel_item_flow(bot, item_name)
    progress(bot, "cancel_item_flow item=" .. item_name)
    local packet, err = open_cancel_list_expect_present(bot)
    if packet == nil then
        return nil, err
    end

    if packet.type == "normal" then
        local msg = packet.message
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return msg, nil
    end

    progress(bot, "cancel select ITEM name=" .. item_name)
    packet = bot:request_dialog_ext(
        protocol.dialog("ITEM", 0, "", 0, 0, item_name, "NEXT"),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "cancel_confirm", packet)
    if packet == nil then
        return nil, "cancel confirm missing"
    end

    if packet.type == "list" and message_contains(packet, MSG_CANCEL_CONFIRM) == false then
        packet = bot:request_dialog_ext(
            protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"),
            function(p)
                return p.type == "list" or p.type == "normal"
            end)
        dump_dialog(bot, "cancel_confirm_multi", packet)
        if packet == nil then
            return nil, "cancel confirm after multi missing"
        end
    end

    if packet.type == "normal" then
        local msg = packet.message
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return msg, nil
    end

    progress(bot, "cancel confirm YES")
    packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", CONFIRM_YES, 0, "", "NEXT"),
        function(p)
            return p.type == "normal" and p.message ~= nil
        end)
    dump_dialog(bot, "cancel_result", packet)
    if packet == nil then
        return nil, "cancel result missing"
    end

    local msg = packet.message
    local next_packet = dismiss_normal_to_list(bot)
    if next_packet ~= nil and next_packet.type == "normal" then
        dismiss_normal_to_list(bot)
    elseif next_packet ~= nil and next_packet.type == "item" then
        close_marketplace_menu(bot)
        return msg, nil
    end
    close_marketplace_menu(bot)
    progress(bot, "cancel_item_flow done msg=" .. tostring(msg))
    return msg, nil
end

local function assert_cancel_list_empty(bot)
    progress(bot, "assert_cancel_list_empty")
    local packet, err = open_cancel_list_expect_empty(bot)
    if packet == nil then
        return false, tostring(err)
    end
    if message_contains(packet, MSG_CANCEL_EMPTY) == false and is_cancel_empty_message(packet) == false then
        local msg = packet.message
        dismiss_normal_to_list(bot)
        close_marketplace_menu(bot)
        return false, "expected empty cancel list, got=" .. tostring(msg)
    end

    dismiss_normal_to_list(bot)
    close_marketplace_menu(bot)
    return true, nil
end

-- bot:money() is a client cache; during HTTP delay the server pushes update_internal (0x08)
-- before the purchase dialog returns. Observe that packet instead of reading cached money.
local function wait_money_deducted_during_delay(ctx, bot, pay_amount)
    local buyer_name = bot:name()
    local observed_deduct = false
    local observed_money = nil
    local last_update_money = nil
    local last_update_level = nil

    ctx:hook("update_internal", function(_, hooked_bot, packet)
        if hooked_bot:name() ~= buyer_name then
            return
        end
        last_update_money = packet.ch_money
        last_update_level = packet.level
        if g_purchase_api_started == false then
            return
        end
        if g_purchase_msg ~= nil then
            return
        end
        if packet.ch_money ~= nil and packet.ch_money < pay_amount then
            observed_deduct = true
            observed_money = packet.ch_money
            progress(bot, string.format(
                "P2: update_internal during delay money=%d level=%s cache=%d",
                packet.ch_money,
                tostring(packet.level),
                bot:money()))
        end
    end)

    local waited = 0
    while g_purchase_api_started == false and g_purchase_msg == nil do
        ctx:sleep(100)
        waited = waited + 100
    end
    if g_purchase_api_started == false then
        ctx:unhook("update_internal")
        progress(bot, "FAILED: purchase confirm was never reached")
        return false, nil
    end

    progress(bot, string.format(
        "P2: purchase API started, observing delay pay=%d cache=%d",
        pay_amount,
        bot:money()))

    waited = 0
    while observed_deduct == false and g_purchase_msg == nil do
        ctx:sleep(100)
        waited = waited + 100
    end

    ctx:unhook("update_internal")

    progress(bot, string.format(
        "P2: money probe done ok=%s observed=%s cache=%d purchase_done=%s last_update=%s/%s waited=%d",
        tostring(observed_deduct),
        tostring(observed_money),
        bot:money(),
        tostring(g_purchase_msg ~= nil),
        tostring(last_update_level),
        tostring(last_update_money),
        waited))

    return observed_deduct, observed_money
end

test_suite {
    name      = "Marketplace Test",
    bot_count = 2,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "MARKETPLACE TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        local ok, err = assert_cancel_list_empty(ctx:bot(0))
        if ok == false then
            progress(ctx:bot(0), "FAILED: precondition own listings not empty " .. tostring(err))
            error("precondition failed: bot0 own listings not empty")
        end
        ok, err = assert_cancel_list_empty(ctx:bot(1))
        if ok == false then
            progress(ctx:bot(1), "FAILED: precondition own listings not empty " .. tostring(err))
            error("precondition failed: bot1 own listings not empty")
        end
        ok, err = assert_search_has_item(ctx:bot(0), WEAPON_ITEM, false)
        if ok == false then
            progress(ctx:bot(0), "FAILED: precondition weapon listing exists " .. tostring(err))
            error("precondition failed: weapon listing exists")
        end
        ok, err = assert_search_has_item(ctx:bot(0), BUNDLE_ITEM, false)
        if ok == false then
            progress(ctx:bot(0), "FAILED: precondition bundle listing exists " .. tostring(err))
            error("precondition failed: bundle listing exists")
        end
        cleanup_bot(ctx:bot(0))
        cleanup_bot(ctx:bot(1))
    end,

    on_finished = function(ctx)
        cleanup_bot(ctx:bot(0))
        cleanup_bot(ctx:bot(1))
    end,

    scenarios = {
        -- M1: listing fee insufficient
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "M1: FEE FAIL")

            if prepare_weapon(a) == false then
                progress(a, "FAILED: prepare weapon")
                return false
            end
            a:money(0)

            local msg, err = list_item_flow(a, WEAPON_ITEM, 1, WEAPON_LIST_PRICE)
            if msg == nil then
                progress(a, "FAILED: " .. tostring(err))
                return false
            end
            if msg:find(MSG_FEE_FAIL, 1, true) == nil and msg:find(MSG_LIST_FAIL, 1, true) == nil then
                progress(a, "FAILED: unexpected msg=" .. tostring(msg))
                return false
            end
            if a:has_item_by_name(WEAPON_ITEM) == false then
                progress(a, "FAILED: weapon removed on fee fail")
                return false
            end

            progress(a, "M1 PASSED")
            return true
        end,

        -- M2: list weapon successfully
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "M2: LIST WEAPON")

            if a:has_item_by_name(WEAPON_ITEM) == false and prepare_weapon(a) == false then
                progress(a, "FAILED: prepare weapon")
                return false
            end
            a:money(listing_fee(1, WEAPON_LIST_PRICE) + 1000)

            local msg, err = list_item_flow(a, WEAPON_ITEM, 1, WEAPON_LIST_PRICE)
            if msg == nil then
                progress(a, "FAILED: " .. tostring(err))
                return false
            end
            if msg:find(MSG_LIST_OK, 1, true) == nil then
                progress(a, "FAILED: list msg=" .. tostring(msg))
                return false
            end
            if a:has_item_by_name(WEAPON_ITEM) then
                progress(a, "FAILED: weapon still in inventory")
                return false
            end

            local ok, search_err = assert_search_has_item(a, WEAPON_ITEM, true)
            if ok == false then
                progress(a, "FAILED: " .. tostring(search_err))
                return false
            end

            progress(a, "M2 PASSED")
            return true
        end,

        -- M3: list bundle partially
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "M3: LIST BUNDLE")

            if prepare_bundle(b) == false then
                progress(b, "FAILED: prepare bundle")
                return false
            end
            b:money(listing_fee(BUNDLE_LIST_COUNT, BUNDLE_UNIT_PRICE) + 1000)

            local msg, err = list_item_flow(b, BUNDLE_ITEM, BUNDLE_LIST_COUNT, BUNDLE_UNIT_PRICE)
            if msg == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end
            if msg:find(MSG_LIST_OK, 1, true) == nil then
                progress(b, "FAILED: list msg=" .. tostring(msg))
                return false
            end
            if b:item_count(BUNDLE_ITEM) ~= (BUNDLE_PREP_COUNT - BUNDLE_LIST_COUNT) then
                progress(b, string.format("FAILED: leftover count=%d", b:item_count(BUNDLE_ITEM)))
                return false
            end

            progress(b, "M3 PASSED")
            return true
        end,

        -- M4: cancel weapon, receive from storage, relist
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "M4: CANCEL RELIST")

            local msg, err = cancel_item_flow(a, WEAPON_ITEM)
            if msg == nil then
                progress(a, "FAILED: " .. tostring(err))
                return false
            end
            if msg:find(MSG_CANCEL_OK, 1, true) == nil then
                progress(a, "FAILED: cancel msg=" .. tostring(msg))
                return false
            end

            ctx:sleep(MQ_WAIT_MS)
            local ok, recv_err = receive_storage(a, STORAGE_CANCEL_TITLE)
            if ok == false then
                progress(a, "FAILED: storage receive " .. tostring(recv_err))
                return false
            end
            if a:has_item_by_name(WEAPON_ITEM) == false then
                progress(a, "FAILED: weapon not restored")
                return false
            end

            a:money(a:money() + listing_fee(1, WEAPON_LIST_PRICE) + 1000)
            msg, err = list_item_flow(a, WEAPON_ITEM, 1, WEAPON_LIST_PRICE)
            if msg == nil or msg:find(MSG_LIST_OK, 1, true) == nil then
                progress(a, "FAILED: relist " .. tostring(msg or err))
                return false
            end

            progress(a, "M4 PASSED")
            return true
        end,

        -- M5: mutual purchase and clear listings
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "M5: MUTUAL PURCHASE")

            b:money(WEAPON_LIST_PRICE + 1000)
            local msg, err = purchase_item_flow(b, WEAPON_ITEM, 1)
            if msg == nil or msg:find(MSG_PURCHASE_OK, 1, true) == nil then
                progress(b, "FAILED: purchase weapon " .. tostring(msg or err))
                return false
            end

            a:money(BUNDLE_LIST_COUNT * BUNDLE_UNIT_PRICE + 1000)
            msg, err = purchase_item_flow(a, BUNDLE_ITEM, BUNDLE_LIST_COUNT)
            if msg == nil or msg:find(MSG_PURCHASE_OK, 1, true) == nil then
                progress(a, "FAILED: purchase bundle " .. tostring(msg or err))
                return false
            end

            ctx:sleep(MQ_WAIT_MS)
            if receive_storage(b, STORAGE_PURCHASE_TITLE) == false then
                progress(b, "FAILED: buyer weapon storage")
                return false
            end
            if receive_storage(a, STORAGE_PURCHASE_TITLE) == false then
                progress(a, "FAILED: buyer bundle storage")
                return false
            end
            if receive_storage(a, STORAGE_SALE_TITLE) == false then
                progress(a, "FAILED: seller weapon sale storage")
                return false
            end
            if receive_storage(b, STORAGE_SALE_TITLE) == false then
                progress(b, "FAILED: seller bundle sale storage")
                return false
            end

            if b:has_item_by_name(WEAPON_ITEM) == false then
                progress(b, "FAILED: buyer missing weapon")
                return false
            end
            if a:item_count(BUNDLE_ITEM) < BUNDLE_LIST_COUNT then
                progress(a, "FAILED: buyer missing bundle")
                return false
            end

            -- Verify no active listings remain for test items after mutual purchase.
            progress(a, "M5: verify listings cleared from search")

            local ok, search_err = assert_search_has_item(a, WEAPON_ITEM, false)
            if ok == false then
                progress(a, "FAILED: weapon still listed " .. tostring(search_err))
                return false
            end
            ok, search_err = assert_search_has_item(a, BUNDLE_ITEM, false)
            if ok == false then
                progress(a, "FAILED: bundle still listed " .. tostring(search_err))
                return false
            end

            cleanup_bot(a)
            cleanup_bot(b)
            progress(a, "M5 PASSED")
            return true
        end,

        -- P1 prep
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "P1 PREP: LIST HTTP DELAY")

            cleanup_bot(a)
            cleanup_bot(b)
            if prepare_weapon(a) == false then
                progress(a, "FAILED: prepare weapon")
                return false
            end
            a:money(listing_fee(1, WEAPON_LIST_PRICE) + 1000)
            a:chat(string.format("/HTTP지연 %d", HTTP_DELAY_MS))
            g_list_msg = nil
            g_search_ok = false
            g_search_err = nil
            progress(a, "P1 PREP DONE")
            return true
        end,

        -- P1 parallel: list + search during delay
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local bot = ctx:bot(0)
                        progress(bot, "P1: CONFIRM LIST")
                        g_list_msg = select(1, list_item_flow(bot, WEAPON_ITEM, 1, WEAPON_LIST_PRICE))
                        progress(bot, "P1: LIST DONE msg=" .. tostring(g_list_msg))
                        return g_list_msg ~= nil and g_list_msg:find(MSG_LIST_OK, 1, true) ~= nil
                    end,
                },
                [1] = {
                    function(ctx)
                        local bot = ctx:bot(1)
                        local seller = ctx:bot(0)
                        progress(bot, "P1: SEARCH DURING DELAY")
                        ctx:sleep(DELAY_PROBE_MS)
                        if seller:has_item_by_name(WEAPON_ITEM) then
                            g_search_err = "seller still holds item during delay"
                            progress(bot, "FAILED: " .. g_search_err)
                            return false
                        end
                        g_search_ok, g_search_err = assert_search_has_item(bot, WEAPON_ITEM, true)
                        if g_search_ok == false then
                            progress(bot, "FAILED: " .. tostring(g_search_err))
                            return false
                        end
                        progress(bot, "P1: SEARCH FOUND DURING DELAY")
                        return true
                    end,
                },
            },
        },

        -- P1 verify + cleanup
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            a:chat("/HTTP지연 0")

            if g_list_msg == nil or g_list_msg:find(MSG_LIST_OK, 1, true) == nil then
                progress(a, "FAILED: list " .. tostring(g_list_msg))
                return false
            end
            if g_search_ok == false then
                progress(a, "FAILED: mid-delay search " .. tostring(g_search_err))
                return false
            end

            cancel_item_flow(a, WEAPON_ITEM)
            ctx:sleep(MQ_WAIT_MS)
            receive_storage(a, STORAGE_CANCEL_TITLE)
            cleanup_bot(a)
            cleanup_bot(b)
            progress(a, "P1 PASSED")
            return true
        end,

        -- P2 prep
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "P2 PREP: PURCHASE HTTP DELAY")

            cleanup_bot(a)
            cleanup_bot(b)
            if prepare_weapon(a) == false then
                progress(a, "FAILED: prepare weapon")
                return false
            end
            a:money(listing_fee(1, WEAPON_LIST_PRICE) + 1000)
            local msg = list_item_flow(a, WEAPON_ITEM, 1, WEAPON_LIST_PRICE)
            if msg == nil or msg:find(MSG_LIST_OK, 1, true) == nil then
                progress(a, "FAILED: setup list")
                return false
            end

            g_pay_amount = WEAPON_LIST_PRICE + 1000
            b:money(g_pay_amount)
            b:chat(string.format("/HTTP지연 %d", HTTP_DELAY_MS))
            g_purchase_msg = nil
            g_money_ok = false
            g_purchase_api_started = false
            progress(a, "P2 PREP DONE")
            return true
        end,

        {
            parallel = {
                [0] = {
                    function(ctx)
                        local bot = ctx:bot(1)
                        progress(bot, "P2: CONFIRM PURCHASE")
                        g_purchase_msg = select(1, purchase_item_flow(bot, WEAPON_ITEM, 1))
                        progress(bot, "P2: PURCHASE DONE msg=" .. tostring(g_purchase_msg))
                        return g_purchase_msg ~= nil and g_purchase_msg:find(MSG_PURCHASE_OK, 1, true) ~= nil
                    end,
                },
                [1] = {
                    function(ctx)
                        local bot = ctx:bot(1)
                        progress(bot, "P2: CHECK MONEY DURING DELAY")
                        local ok, observed_money = wait_money_deducted_during_delay(ctx, bot, g_pay_amount)
                        g_money_ok = ok == true
                        if g_money_ok == false then
                            progress(bot, string.format(
                                "P2: no update_internal deduction observed (observed=%s cache=%d pay=%d)",
                                tostring(observed_money),
                                bot:money(),
                                g_pay_amount))
                        end
                        return g_money_ok
                    end,
                },
            },
        },

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            b:chat("/HTTP지연 0")

            if g_purchase_msg == nil or g_purchase_msg:find(MSG_PURCHASE_OK, 1, true) == nil then
                progress(b, "FAILED: purchase " .. tostring(g_purchase_msg))
                return false
            end
            if g_money_ok == false then
                progress(b, string.format(
                    "FAILED: money not deducted during delay cache=%d pay=%d",
                    b:money(),
                    g_pay_amount))
                return false
            end

            progress(b, string.format(
                "P2: verify after purchase cache=%d expected<=%d",
                b:money(),
                g_pay_amount - WEAPON_LIST_PRICE))

            ctx:sleep(MQ_WAIT_MS)
            receive_storage(b, STORAGE_PURCHASE_TITLE)
            receive_storage(a, STORAGE_SALE_TITLE)
            cleanup_bot(a)
            cleanup_bot(b)
            progress(a, "P2 PASSED")
            return true
        end,

        -- P4 prep
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "P4 PREP: PURCHASE VS CANCEL RACE")

            cleanup_bot(a)
            cleanup_bot(b)
            if prepare_weapon(a) == false then
                progress(a, "FAILED: prepare weapon")
                return false
            end
            a:money(listing_fee(1, WEAPON_LIST_PRICE) + 1000)
            local msg = list_item_flow(a, WEAPON_ITEM, 1, WEAPON_LIST_PRICE)
            if msg == nil or msg:find(MSG_LIST_OK, 1, true) == nil then
                progress(a, "FAILED: setup list")
                return false
            end

            b:money(WEAPON_LIST_PRICE + 1000)
            a:chat(string.format("/HTTP지연 %d", HTTP_DELAY_MS))
            b:chat(string.format("/HTTP지연 %d", HTTP_DELAY_MS))
            g_purchase_msg = nil
            g_cancel_msg = nil
            progress(a, "P4 PREP DONE")
            return true
        end,

        {
            parallel = {
                [0] = {
                    function(ctx)
                        local bot = ctx:bot(1)
                        progress(bot, "P4: BUYER PURCHASE")
                        g_purchase_msg = select(1, purchase_item_flow(bot, WEAPON_ITEM, 1))
                        progress(bot, "P4: purchase msg=" .. tostring(g_purchase_msg))
                        return true
                    end,
                },
                [1] = {
                    function(ctx)
                        local bot = ctx:bot(0)
                        progress(bot, "P4: SELLER CANCEL")
                        ctx:sleep(200)
                        g_cancel_msg = select(1, cancel_item_flow(bot, WEAPON_ITEM))
                        progress(bot, "P4: cancel msg=" .. tostring(g_cancel_msg))
                        return true
                    end,
                },
            },
        },

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            a:chat("/HTTP지연 0")
            b:chat("/HTTP지연 0")

            local purchase_ok = g_purchase_msg ~= nil and g_purchase_msg:find(MSG_PURCHASE_OK, 1, true) ~= nil
            local cancel_ok = g_cancel_msg ~= nil and g_cancel_msg:find(MSG_CANCEL_OK, 1, true) ~= nil
            if purchase_ok == cancel_ok then
                progress(a, string.format(
                    "FAILED: expected exactly one winner purchase=%s cancel=%s p=%s c=%s",
                    tostring(purchase_ok),
                    tostring(cancel_ok),
                    tostring(g_purchase_msg),
                    tostring(g_cancel_msg)))
                return false
            end

            ctx:sleep(MQ_WAIT_MS)
            if purchase_ok then
                receive_storage(b, STORAGE_PURCHASE_TITLE)
                receive_storage(a, STORAGE_SALE_TITLE)
                if b:has_item_by_name(WEAPON_ITEM) == false then
                    progress(b, "FAILED: purchase winner missing item")
                    return false
                end
            else
                receive_storage(a, STORAGE_CANCEL_TITLE)
                if a:has_item_by_name(WEAPON_ITEM) == false then
                    progress(a, "FAILED: cancel winner missing item")
                    return false
                end
            end

            local ok, search_err = assert_search_has_item(a, WEAPON_ITEM, false)
            if ok == false then
                progress(a, "FAILED: listing remains " .. tostring(search_err))
                return false
            end

            cleanup_bot(a)
            cleanup_bot(b)
            progress(a, "P4 PASSED")
            return true
        end,
    },
}
