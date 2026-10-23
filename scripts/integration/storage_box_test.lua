local lib      = require("integration.lib")
local protocol = require("integration.protocol")

local F1_OID           = 0xFFFFFFFF
local DIALOG_CLOSE_OID = 0xFFFFFFFD
local OPT_STORAGE      = "통합보관함"
local OPT_GRANT        = "아이템 지급"
local OPT_TARGET_USER  = "특정 유저"
local OPT_REWARD_ITEM  = "아이템"
local OPT_REWARD_NONE  = "없음"
local OPT_YES          = "예"

-- NORMAL dialog resumes with request.action (not button).
-- Must match fb::model::enum_value::DIALOG_RESULT.
local DIALOG_PREV = 0
local DIALOG_NEXT = 2

local REWARD_ITEM_NAME  = "도토리"
local REWARD_ITEM_COUNT = 1
local FILL_ITEM_NAME    = "목도"
local MQ_WAIT_MS        = 2000
local HTTP_DELAY_MS     = 3000
local C2_FILL_WAIT_MS   = 1000
local CLAIM_PATH        = "/storage/claim"

local MSG_GRANT_NEED_REWARD = "보상이 하나 이상 필요합니다."
local MSG_GRANT_OK          = "지급이 완료되었습니다."
local MSG_RECEIVE_OK        = "보상이 지급되었습니다."
local MSG_RECEIVE_FULL      = "소지품이 가득 차서 보상을 받을 수 없습니다"
local MSG_RECEIVE_PENDING   = "수령 결과를 확인하지 못했습니다"
local MSG_STATUS_AVAILABLE  = "수령 가능"
local MSG_STATUS_RECEIVED   = "수령 완료"

local TITLE   = nil
local MESSAGE = nil

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("storage_box_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function cleanup_bot(bot)
    bot:chat("/아이템초기화")
    bot:chat("/아이템삭제")
    bot:money(0)
    bot:chat("/HTTP지연 0")
    bot:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
    bot:chat("/HTTP보류 off")
end

local function find_menu_option(packet, text)
    if packet == nil then
        return nil
    end
    local menus = packet.menu_menus or packet.list_lists
    if menus == nil then
        return nil
    end
    for _, name in ipairs(menus) do
        if name == text then
            return name
        end
    end
    return nil
end

local function message_contains(packet, text)
    return packet ~= nil
        and packet.message ~= nil
        and packet.message:find(text, 1, true) ~= nil
end

local function dialog_normal_next()
    return protocol.dialog("NORMAL", DIALOG_NEXT, "", 0, 0, "", "NEXT")
end

local function dialog_normal_prev()
    return protocol.dialog("NORMAL", DIALOG_PREV, "", 0, 0, "", "PREV")
end

local function pursuit_select(option)
    return protocol.dialog("PURSUIT", 0, "", 0, 0, option)
end

-- TOP releases any waiting 0x2F/0x30 dialog without depending on the next script goto.
local function close_dialog(bot)
    bot:send(protocol.click(DIALOG_CLOSE_OID))
end

local function f1_open_menu(bot)
    return bot:request_dialog(
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "pursuit"
        end)
end

local function f1_select_pursuit(bot, option, expect_type)
    expect_type = expect_type or "pursuit"
    local use_ext = (expect_type == "list" or expect_type == "normal" or expect_type == "input_ext")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    return fn(bot,
        pursuit_select(option),
        function(packet)
            return packet.type == expect_type
        end)
end

local function send_input_expect_input(bot, text)
    return bot:request_dialog(
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "input"
        end)
end

local function send_input_expect_pursuit(bot, text)
    return bot:request_dialog(
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "pursuit"
        end)
end

local function grant_flow(bot_a, target_name, title, message, item_name, item_count)
    local packet = f1_open_menu(bot_a)
    if packet == nil then
        return false, "f1 open failed"
    end

    packet = f1_select_pursuit(bot_a, OPT_GRANT, "pursuit")
    if packet == nil or message_contains(packet, "누구에게") == false then
        return false, "grant menu missing"
    end

    packet = f1_select_pursuit(bot_a, OPT_TARGET_USER, "input")
    if packet == nil then
        return false, "name input missing"
    end

    packet = send_input_expect_input(bot_a, target_name)
    if packet == nil then
        return false, "title input missing"
    end

    packet = send_input_expect_input(bot_a, title)
    if packet == nil then
        return false, "message input missing"
    end

    packet = send_input_expect_pursuit(bot_a, message)
    if packet == nil then
        return false, "reward menu missing"
    end

    if item_name ~= nil then
        packet = f1_select_pursuit(bot_a, OPT_REWARD_ITEM, "input")
        if packet == nil then
            return false, "item name input missing"
        end

        packet = send_input_expect_input(bot_a, item_name)
        if packet == nil then
            return false, "item count input missing"
        end

        packet = send_input_expect_pursuit(bot_a, tostring(item_count))
        if packet == nil then
            return false, "reward menu after item missing"
        end
    end

    packet = bot_a:request_dialog_ext(
        pursuit_select(OPT_REWARD_NONE),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return false, "grant result dialog missing"
    end

    local result_msg = packet.message
    -- grant_storage returns to F1; TOP is enough because later steps reopen F1.
    close_dialog(bot_a)
    return true, result_msg
end

local function open_storage_entry_detail(bot, title)
    local packet = f1_open_menu(bot)
    if packet == nil then
        return nil, "f1 open failed"
    end

    packet = f1_select_pursuit(bot, OPT_STORAGE, "pursuit")
    if packet == nil then
        return nil, "storage list missing"
    end

    local option = find_menu_option(packet, title)
    if option == nil then
        close_dialog(bot)
        return nil, "entry not found: " .. title
    end

    packet = bot:request_dialog_ext(
        pursuit_select(option),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return nil, "entry detail missing"
    end
    return packet, nil
end

local function close_detail_with_prev(bot)
    -- PREV returns to STORAGE_LIST; entry still exists so response is pursuit.
    bot:request_dialog(
        dialog_normal_prev(),
        function(p)
            return p.type == "pursuit"
        end)
    close_dialog(bot)
end

local function dump_dialog(bot, label, packet)
    if packet == nil then
        progress(bot, string.format("DIALOG[%s] nil", label))
        return
    end
    local lists = ""
    if packet.menu_menus ~= nil then
        lists = table.concat(packet.menu_menus, "|")
    elseif packet.list_lists ~= nil then
        lists = table.concat(packet.list_lists, "|")
    end
    progress(bot, string.format(
        "DIALOG[%s] type=%s msg=%q lists=%q",
        label,
        tostring(packet.type),
        tostring(packet.message),
        lists))
end

local function receive_from_detail(bot, expect_text)
    progress(bot, string.format("receive_from_detail invent_size=%s expect=%q",
        tostring(bot:inventory_size()),
        expect_text))

    local packet = bot:request_dialog(
        dialog_normal_next(),
        function(p)
            return p.type == "pursuit"
        end)
    dump_dialog(bot, "after-detail-next", packet)
    if packet == nil or packet.type ~= "pursuit" then
        return false, "receive confirm missing"
    end
    if packet.message == nil or packet.message:find("수령하시겠습니까", 1, true) == nil then
        progress(bot, string.format("UNEXPECTED confirm dialog msg=%q", tostring(packet.message)))
        return false, "receive confirm list missing"
    end

    progress(bot, "receive_from_detail click YES")
    packet = bot:request_dialog_ext(
        pursuit_select(OPT_YES),
        function(p)
            if p.type == "normal" and p.message ~= nil then
                log("debug", string.format(
                    "storage_box_test receive YES candidate type=%s msg=%q",
                    tostring(p.type),
                    tostring(p.message)))
            end
            return p.type == "normal" and p.message ~= nil
        end)
    dump_dialog(bot, "after-receive-yes", packet)
    if packet == nil then
        return false, "receive result missing"
    end

    local matched = message_contains(packet, expect_text)
    local result_msg = packet.message
    progress(bot, string.format("receive_from_detail matched=%s msg=%q", tostring(matched), tostring(result_msg)))
    -- After result, storage.lua goto STORAGE_LIST (empty→normal / remaining→pursuit).
    -- TOP closes without depending on that branch.
    close_dialog(bot)
    return matched, result_msg
end

local function open_receive_confirm(bot, title)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail == nil then
        return nil, err
    end

    local packet = bot:request_dialog(
        dialog_normal_next(),
        function(p)
            return p.type == "pursuit"
        end)
    if packet == nil then
        return nil, "receive confirm missing"
    end
    return packet, nil
end

local function assert_entry_status(bot, title, status_text)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail == nil then
        return false, err
    end
    local ok = message_contains(detail, status_text)
    local msg = detail.message
    close_detail_with_prev(bot)
    return ok, msg
end

local function prepare_one_slot_free(bot)
    bot:clear_inventory()
    bot:fill_inventory(FILL_ITEM_NAME)
    bot:drop_item(0, false)
end

local function wait_item_count(ctx, bot, item_name, expected)
    return lib.wait.settle(ctx, function()
        return bot:item_count(item_name) == expected
    end, function(attempt)
        progress(bot, string.format("reward not settled yet attempt=%d count=%d expected=%d",
            attempt, bot:item_count(item_name), expected))
    end)
end

-- The anchor box keeps the storage list non-empty, so a missing title means the box is no longer listed.
local function assert_entry_gone(bot, title)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail ~= nil then
        close_detail_with_prev(bot)
        return false, "entry still listed: " .. title
    end
    if err ~= "entry not found: " .. title then
        return false, err
    end
    return true, nil
end

-- Grants a fresh box and fails its first receive under the given fault, leaving the claim unsettled.
local function receive_under_fault(ctx, a, b, title, fault_command)
    b:clear_inventory()
    local ok, msg = grant_flow(a, b:name(), title, MESSAGE, REWARD_ITEM_NAME, REWARD_ITEM_COUNT)
    if ok == false or msg == nil or msg:find(MSG_GRANT_OK, 1, true) == nil then
        return false, "grant " .. tostring(msg)
    end
    ctx:sleep(MQ_WAIT_MS)

    b:chat(fault_command)
    local detail, err = open_storage_entry_detail(b, title)
    if detail == nil then
        return false, err
    end
    local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_PENDING)
    if matched == false then
        return false, "expected pending receive, got=" .. tostring(result_msg)
    end
    if b:item_count(REWARD_ITEM_NAME) ~= 0 then
        return false, "reward granted while the claim was unknown"
    end
    return true, nil
end

-- A later tick must not grant the same box again.
local function assert_granted_once(ctx, bot, title)
    lib.wait.settle_tick(ctx)
    if bot:item_count(REWARD_ITEM_NAME) ~= REWARD_ITEM_COUNT then
        return false, string.format("reward duplicated count=%d", bot:item_count(REWARD_ITEM_NAME))
    end
    return assert_entry_gone(bot, title)
end

test_suite {
    name      = "Storage Box Test",
    bot_count = 2,

    on_initialize = function(ctx)
        local a = ctx:bot(0)
        local b = ctx:bot(1)
        TITLE   = string.format("SB_%s_%s", a:name(), b:name())
        MESSAGE = "SB_TEST_MSG"
        progress(a, "STORAGE BOX TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        cleanup_bot(a)
        cleanup_bot(b)
    end,

    on_scenario_finished = function(ctx)
        lib.wait.resume_settle(ctx:bot(0))
    end,

    on_finished = function(ctx)
        cleanup_bot(ctx:bot(0))
        cleanup_bot(ctx:bot(1))
    end,

    scenarios = {
        -- S1: grant without reward must fail
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "S1: GRANT WITHOUT REWARD")

            local ok, msg = grant_flow(a, b:name(), TITLE .. "_NOREWARD", MESSAGE, nil, nil)
            if ok == false then
                progress(a, "FAILED: " .. tostring(msg))
                return false
            end
            if msg == nil or msg:find(MSG_GRANT_NEED_REWARD, 1, true) == nil then
                progress(a, "FAILED: expected need-reward message, got=" .. tostring(msg))
                return false
            end

            progress(a, "S1 PASSED")
            return true
        end,

        -- S2: personal grant success
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "S2: PERSONAL GRANT")

            local ok, msg = grant_flow(a, b:name(), TITLE, MESSAGE, REWARD_ITEM_NAME, REWARD_ITEM_COUNT)
            if ok == false then
                progress(a, "FAILED: " .. tostring(msg))
                return false
            end
            if msg == nil or msg:find(MSG_GRANT_OK, 1, true) == nil then
                progress(a, "FAILED: expected grant ok, got=" .. tostring(msg))
                return false
            end

            progress(b, "S2: WAIT MQ DISTRIBUTE")
            ctx:sleep(MQ_WAIT_MS)

            local detail, err = open_storage_entry_detail(b, TITLE)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end
            if message_contains(detail, MSG_STATUS_AVAILABLE) == false then
                progress(b, "FAILED: entry not available, msg=" .. tostring(detail.message))
                close_detail_with_prev(b)
                return false
            end
            close_detail_with_prev(b)

            progress(a, "S2 PASSED")
            return true
        end,

        -- S3/C1: invent full -> claim not attempted, still available
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S3/C1: RECEIVE WITH FULL INVENTORY")

            b:fill_inventory(FILL_ITEM_NAME)
            progress(b, string.format("S3/C1 after fill invent_size=%d", b:inventory_size()))
            if b:inventory_size() < 52 then
                progress(b, string.format("FAILED: invent not full after fill, size=%d", b:inventory_size()))
                return false
            end

            local detail, err = open_storage_entry_detail(b, TITLE)
            dump_dialog(b, "S3-detail", detail)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            -- Hook server-side notify messages during receive
            local notifies = {}
            ctx:hook("message", function(_, hooked_bot, packet)
                if hooked_bot:name() ~= b:name() then
                    return
                end
                if packet.text ~= nil then
                    table.insert(notifies, packet.text)
                    log("debug", string.format("storage_box_test S3 message: %q", packet.text))
                end
            end)

            local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_FULL)
            ctx:unhook("message")
            progress(b, string.format("S3/C1 notifies=%s", table.concat(notifies, " || ")))

            if matched == false then
                progress(b, "FAILED: expected receive fail, got=" .. tostring(result_msg))
                return false
            end

            local status_ok, status_msg = assert_entry_status(b, TITLE, MSG_STATUS_AVAILABLE)
            if status_ok == false then
                progress(b, "FAILED: expected still available, got=" .. tostring(status_msg))
                return false
            end

            progress(b, "S3/C1 PASSED")
            return true
        end,

        -- S4/C2 prep: leave one free slot, arm HTTP delay, open receive confirm
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S4/C2 PREP: ONE SLOT FREE + HTTP DELAY + CONFIRM")

            prepare_one_slot_free(b)
            b:chat(string.format("/HTTP지연 %d", HTTP_DELAY_MS))
            ctx:sleep(200)

            local confirm, err = open_receive_confirm(b, TITLE)
            if confirm == nil then
                progress(b, "FAILED: " .. tostring(err))
                b:chat("/HTTP지연 0")
                return false
            end

            progress(b, "S4/C2 PREP DONE")
            return true
        end,

        -- S4/C2: click yes while peer task fills last slot during HTTP delay
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "S4/C2: CLICK RECEIVE YES")

                        local packet = b:request_dialog_ext(
                            pursuit_select(OPT_YES),
                            function(p)
                                if p.type == "normal" and p.message ~= nil then
                                    log("debug", string.format(
                                        "storage_box_test C2 YES candidate type=%s msg=%q",
                                        tostring(p.type),
                                        tostring(p.message)))
                                end
                                return p.type == "normal" and p.message ~= nil
                            end)

                        dump_dialog(b, "C2-after-yes", packet)
                        progress(b, string.format("C2 invent_size after receive=%d", b:inventory_size()))
                        b:chat("/HTTP지연 0")

                        if packet == nil then
                            progress(b, "FAILED: receive result missing")
                            return false
                        end
                        if message_contains(packet, MSG_RECEIVE_FULL) == false then
                            progress(b, "FAILED: expected receive fail after unclaim, got=" .. tostring(packet.message))
                            close_dialog(b)
                            return false
                        end

                        close_dialog(b)

                        local status_ok, status_msg = assert_entry_status(b, TITLE, MSG_STATUS_AVAILABLE)
                        if status_ok == false then
                            progress(b, "FAILED: expected unclaim available, got=" .. tostring(status_msg))
                            return false
                        end

                        progress(b, "S4/C2 PASSED")
                        return true
                    end,
                },
                [1] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "S4/C2: FILL LAST SLOT DURING DELAY")
                        ctx:sleep(C2_FILL_WAIT_MS)
                        b:create_item(FILL_ITEM_NAME, 1)
                        progress(b, "S4/C2: LAST SLOT FILLED")
                        return true
                    end,
                },
            },
        },

        -- S5: happy path receive
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S5: HAPPY PATH RECEIVE")

            b:clear_inventory()
            b:chat("/HTTP지연 0")

            local detail, err = open_storage_entry_detail(b, TITLE)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_OK)
            if matched == false then
                progress(b, "FAILED: expected receive ok, got=" .. tostring(result_msg))
                return false
            end

            if b:has_item_by_name(REWARD_ITEM_NAME) == false then
                progress(b, "FAILED: reward item missing")
                return false
            end
            if b:item_count(REWARD_ITEM_NAME) < REWARD_ITEM_COUNT then
                progress(b, "FAILED: reward count mismatch")
                return false
            end

            -- storage_entries() omits received boxes, so the entry should disappear from the list.
            local packet = f1_open_menu(b)
            if packet == nil then
                progress(b, "FAILED: f1 open after receive")
                return false
            end
            -- Empty storage returns normal; remaining entries return pursuit.
            packet = f1_select_pursuit(b, OPT_STORAGE, "normal")
            dump_dialog(b, "S5-storage-after-receive", packet)
            if packet ~= nil and packet.type == "pursuit" and find_menu_option(packet, TITLE) ~= nil then
                progress(b, "FAILED: received entry still listed")
                return false
            end
            if packet ~= nil and packet.type == "normal" and message_contains(packet, "보관된 항목이 없습니다") then
                close_dialog(b)
            end

            progress(b, "S5 PASSED")
            return true
        end,

        -- R0: anchor box that stays unreceived until R4 so the storage list is never empty
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(b, "R0: ANCHOR BOX")

            local ok, msg = grant_flow(a, b:name(), TITLE .. "_ANCHOR", MESSAGE, REWARD_ITEM_NAME, REWARD_ITEM_COUNT)
            if ok == false or msg == nil or msg:find(MSG_GRANT_OK, 1, true) == nil then
                progress(a, "FAILED: anchor grant " .. tostring(msg))
                return false
            end
            ctx:sleep(MQ_WAIT_MS)

            progress(b, "R0 PASSED")
            return true
        end,

        -- R1: the claim applied but its response was lost, so the settle timer grants the reward exactly once
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local title = TITLE .. "_R1"
            progress(b, "R1: CLAIM RESPONSE LOST, TIMER SETTLES")

            local ok, err = receive_under_fault(ctx, a, b, title, "/HTTP장애 " .. CLAIM_PATH .. " lost")
            b:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
            if ok == false then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            if wait_item_count(ctx, b, REWARD_ITEM_NAME, REWARD_ITEM_COUNT) == false then
                progress(b, "FAILED: reward not granted by the settle timer")
                return false
            end
            ok, err = assert_granted_once(ctx, b, title)
            if ok == false then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            progress(b, "R1 PASSED")
            return true
        end,

        -- R2: the unsettled request of a lost claim survives relogin and is settled on login
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local title = TITLE .. "_R2"
            progress(b, "R2: CLAIM RESPONSE LOST, RELOGIN SETTLES")

            local ok, err = receive_under_fault(ctx, a, b, title, "/HTTP장애 " .. CLAIM_PATH .. " lost")
            if ok == false then
                b:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            lib.wait.pause_settle(a)
            if b:logout() == false then
                a:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
                progress(b, "FAILED: logout")
                return false
            end
            a:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
            if b:login() == false then
                progress(b, "FAILED: relogin")
                return false
            end
            b = ctx:bot(1)

            -- The settle timer is paused, so the grant comes from the login settle.
            ctx:sleep(MQ_WAIT_MS * 2)
            lib.wait.resume_settle(a)
            if b:item_count(REWARD_ITEM_NAME) ~= REWARD_ITEM_COUNT then
                progress(b, string.format("FAILED: reward not granted after relogin count=%d",
                    b:item_count(REWARD_ITEM_NAME)))
                return false
            end
            ok, err = assert_granted_once(ctx, b, title)
            if ok == false then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            lib.formation.arrange_in_line(ctx)
            progress(b, "R2 PASSED")
            return true
        end,

        -- R3: the claim never reached internal, so the settle resends the same claim id and grants once
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local title = TITLE .. "_R3"
            progress(b, "R3: CLAIM UNREACHABLE, RESEND GRANTS")

            local ok, err = receive_under_fault(ctx, a, b, title, "/HTTP장애 " .. CLAIM_PATH .. " on")
            if ok == false then
                b:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            -- The unsettled request must outlive a tick that still cannot reach internal.
            lib.wait.settle_tick(ctx)
            if b:item_count(REWARD_ITEM_NAME) ~= 0 then
                b:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
                progress(b, "FAILED: reward granted while internal was unreachable")
                return false
            end

            b:chat("/HTTP장애 " .. CLAIM_PATH .. " off")
            if wait_item_count(ctx, b, REWARD_ITEM_NAME, REWARD_ITEM_COUNT) == false then
                progress(b, "FAILED: reward not granted after internal came back")
                return false
            end
            ok, err = assert_granted_once(ctx, b, title)
            if ok == false then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            progress(b, "R3 PASSED")
            return true
        end,

        -- R4: claims arriving late, after the settle already resent the same id, never grant twice
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local title = TITLE .. "_R4"
            progress(b, "R4: LATE DUPLICATE CLAIMS")

            local ok, err = receive_under_fault(ctx, a, b, title, "/HTTP보류 on " .. CLAIM_PATH)
            if ok == false then
                b:chat("/HTTP보류 off")
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            -- The settle resend is held too, then both copies arrive together.
            lib.wait.settle_tick(ctx)
            b:chat("/HTTP보류 send")

            if wait_item_count(ctx, b, REWARD_ITEM_NAME, REWARD_ITEM_COUNT) == false then
                progress(b, "FAILED: reward not granted after late claims")
                return false
            end
            ok, err = assert_granted_once(ctx, b, title)
            if ok == false then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            b:clear_inventory()
            local detail, detail_err = open_storage_entry_detail(b, TITLE .. "_ANCHOR")
            if detail == nil then
                progress(b, "FAILED: anchor " .. tostring(detail_err))
                return false
            end
            local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_OK)
            if matched == false then
                progress(b, "FAILED: anchor receive " .. tostring(result_msg))
                return false
            end

            progress(b, "R4 PASSED")
            return true
        end,
    },
}
