local lib      = require("integration.lib")
local protocol = require("integration.protocol")

local NPC_NAME = "문파대리인"
local OFF_MAP  = "가상계"
local HOME_MAP = "낙랑의방"
local HOME_Y   = 6

-- Top-level category menu (matches npc/89.lua on_click list)
local CAT_INFO      = 1
local CAT_OPERATION = 2
local CAT_DIPLOMACY = 3
local CAT_LEAVE     = 4

-- clan_operation submenu
local OP_TITLE   = 1
local OP_INVITE  = 2
local OP_KICK    = 3
local OP_ROLE    = 4
local OP_MESSAGE = 5
local OP_DESTROY = 6

-- clan_info submenu
local INFO_CASTLE  = 1
local INFO_MEMBERS = 2
local INFO_ALLY    = 3
local INFO_ENEMY   = 4

-- clan_diplomacy submenu
local DIP_ALLY    = 1
local DIP_UNALLY  = 2
local DIP_ENEMY   = 3
local DIP_UNENEMY = 4

local CONFIRM_YES = 1
local CONFIRM_NO  = 2

local ROLE_DEPUTY = "2"
local ROLE_MASTER = "3"

local MSG_CREATE_OK       = "축하하오"
local MSG_CREATE_DUP      = "클랜명이 이미 존재함"
local MSG_TITLE_OK        = "칭호를 새로 내걸었소"
local MSG_TITLE_PRIV      = "문파 권한이 부족합니다"
local MSG_TITLE_SHORT     = "문파 칭호가 너무 짧습니다"
local MSG_TITLE_SAME      = "문파 칭호가 변경되지 않았습니다"
local MSG_ERR_CODE_33     = "에러코드 : 33"
local MSG_DESTROY_OK      = "문파 해체 수속이 처리되었소"
local MSG_DESTROY_MEMBERS = "문파에 다른 멤버가 존재합니다"
local MSG_NOT_NEAR        = "이 자리에 보이지 않아"
local MSG_REJECTED        = "가 거절했습니다"
local MSG_ACCEPTED        = "가입 수속이 처리되었소"
local MSG_JOINED          = "문파에 가입되었습니다"
local MSG_ALREADY         = "클랜 이미 있음"
local MSG_LEAVE_OK        = "탈퇴 수속이 완료되었소"
local MSG_LEAVE_MASTER    = "문파 마스터는 탈퇴할 수 없습니다"
local MSG_KICK_OK         = "축출 수속이 처리되었소"
local MSG_ROLE_OK         = "직책 임명 수속이 처리되었소"
local MSG_NO_PRIV         = "문파 권한이 부족합니다"
local MSG_ERR_CODE_27     = "에러코드 : 27"
local MSG_WAITING         = "잠시만 기다려보시오"
local MSG_NONE            = "없음"

local MSG_ALLY_OK    = "동맹이 성공적으로 체결되었습니다"
local MSG_UNALLY_OK  = "동맹파기가 완료되었습니다"
local MSG_ENEMY_OK   = "적대관계 설정이 완료되었습니다"
local MSG_UNENEMY_OK = "종전 협약이 완료되었습니다"

local LISTENER_ARM_MS = 500

-- Shared across parallel steps
local g_npc         = nil
local g_invite_ok   = false
local g_invite_err  = nil
local g_invitee_msg = nil
local g_inviter_msg = nil

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("clan_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function message_contains(packet, text)
    return packet ~= nil
        and packet.message ~= nil
        and packet.message:find(text, 1, true) ~= nil
end

local function is_waiting_dialog(msg)
    return msg ~= nil and msg:find(MSG_WAITING, 1, true) ~= nil
end

local function home_x(bot_index)
    return 6 + bot_index
end

local function setup_npc(bot)
    bot:direction("BOTTOM")
    bot:move("BOTTOM")
    local npc = bot:create_npc(NPC_NAME)
    bot:move("TOP")
    bot:direction("BOTTOM")
    g_npc = npc
    progress(bot, string.format("NPC created oid=%s", tostring(npc.oid)))
    return npc
end

local function dismiss_normal(bot)
    -- Final me:dialog has no follow-up packet; do not wait (timeout would fail the scenario).
    bot:send(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"))
end

local function click_expect(bot, expect_type)
    local npc = g_npc
    if npc == nil then
        npc = setup_npc(bot)
    end
    local use_ext = (expect_type == "list" or expect_type == "normal" or expect_type == "input_ext")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    local packet = fn(bot,
        protocol.click(npc.oid),
        function(p)
            return p.type == expect_type
        end)
    return packet
end

local function select_list(bot, index, expect_types)
    expect_types = expect_types or { "list", "normal" }
    local packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            for _, t in ipairs(expect_types) do
                if p.type == t then
                    return true
                end
            end
            return false
        end)
    return packet
end

local function select_list_expect_input(bot, index)
    return bot:request_dialog(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "input"
        end)
end

local function send_input_expect(bot, text, expect_type)
    expect_type = expect_type or "normal"
    local use_ext = (expect_type == "list" or expect_type == "normal" or expect_type == "input_ext")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    return fn(bot,
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(p)
            return p.type == expect_type
        end)
end

local function open_clan_menu(bot)
    local packet = click_expect(bot, "list")
    if packet == nil then
        return nil, "clan menu list missing"
    end
    return packet, nil
end

local function open_category(bot, category)
    local packet, err = open_clan_menu(bot)
    if packet == nil then
        return nil, err
    end
    packet = select_list(bot, category, { "list" })
    if packet == nil then
        return nil, "category list missing"
    end
    return packet, nil
end

local function create_clan(bot, clan_name)
    progress(bot, "create_clan name=" .. clan_name)
    local packet = click_expect(bot, "list")
    if packet == nil then
        return nil, "create confirm missing"
    end
    packet = select_list_expect_input(bot, CONFIRM_YES)
    if packet == nil then
        return nil, "create input missing"
    end
    packet = send_input_expect(bot, clan_name, "normal")
    if packet == nil then
        return nil, "create result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function change_title(bot, title)
    progress(bot, "change_title title=" .. tostring(title))
    local packet, err = open_category(bot, CAT_OPERATION)
    if packet == nil then
        return nil, err
    end
    packet = select_list_expect_input(bot, OP_TITLE)
    if packet == nil then
        return nil, "title input missing"
    end
    packet = send_input_expect(bot, title, "normal")
    if packet == nil then
        return nil, "title result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function destroy_clan(bot)
    progress(bot, "destroy_clan")
    local packet, err = open_category(bot, CAT_OPERATION)
    if packet == nil then
        return nil, err
    end
    packet = select_list(bot, OP_DESTROY, { "list" })
    if packet == nil then
        return nil, "destroy confirm missing"
    end
    packet = select_list(bot, CONFIRM_YES, { "normal" })
    if packet == nil then
        return nil, "destroy result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function leave_clan(bot)
    progress(bot, "leave_clan")
    local packet, err = open_clan_menu(bot)
    if packet == nil then
        return nil, err
    end
    packet = select_list(bot, CAT_LEAVE, { "list" })
    if packet == nil then
        return nil, "leave confirm missing"
    end
    packet = select_list(bot, CONFIRM_YES, { "normal" })
    if packet == nil then
        return nil, "leave result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function kick_member(bot, target_name)
    progress(bot, "kick target=" .. target_name)
    local packet, err = open_category(bot, CAT_OPERATION)
    if packet == nil then
        return nil, err
    end
    packet = select_list_expect_input(bot, OP_KICK)
    if packet == nil then
        return nil, "kick input missing"
    end
    packet = send_input_expect(bot, target_name, "normal")
    if packet == nil then
        return nil, "kick result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function invite_start(bot, target_name)
    local packet, err = open_category(bot, CAT_OPERATION)
    if packet == nil then
        return false, err
    end
    packet = select_list_expect_input(bot, OP_INVITE)
    if packet == nil then
        return false, "invite input missing"
    end
    return true, nil
end

local function diplomacy_start(bot, action_index)
    local packet, err = open_category(bot, CAT_DIPLOMACY)
    if packet == nil then
        return false, err
    end
    packet = select_list_expect_input(bot, action_index)
    if packet == nil then
        return false, "diplomacy input missing"
    end
    return true, nil
end

local function fetch_info(bot, info_index)
    local packet, err = open_category(bot, CAT_INFO)
    if packet == nil then
        return nil, err
    end
    packet = select_list(bot, info_index, { "normal" })
    if packet == nil then
        return nil, "info result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function wait_normal(bot)
    return bot:request_dialog_ext(
        protocol.chat(false, "."),
        function(p)
            return p.type == "normal"
        end)
end

local function send_target_name_expect_normal(bot, target_name)
    local packet = send_input_expect(bot, target_name, "normal")
    if packet == nil then
        return nil
    end
    if is_waiting_dialog(packet.message) then
        dismiss_normal(bot)
        return wait_normal(bot)
    end
    return packet
end

local function send_target_name_dismiss_waiting(bot, target_name)
    local packet = send_input_expect(bot, target_name, "normal")
    if packet == nil or is_waiting_dialog(packet.message) == false then
        return false
    end
    dismiss_normal(bot)
    return true
end

local function wait_menu(bot)
    return bot:request_dialog(
        protocol.chat(false, "."),
        function(p)
            return p.type == "menu"
        end)
end

local function reply_menu_expect_normal(bot, index)
    return bot:request_dialog_ext(
        protocol.dialog("MENU", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "normal"
        end)
end

local function is_title_too_short(msg)
    if msg == nil then
        return false
    end
    return msg:find(MSG_TITLE_SHORT, 1, true) ~= nil
        or msg:find(MSG_ERR_CODE_33, 1, true) ~= nil
end

local function is_privilege_error(msg)
    if msg == nil then
        return false
    end
    return msg:find(MSG_NO_PRIV, 1, true) ~= nil
        or msg:find(MSG_ERR_CODE_27, 1, true) ~= nil
end

local function send_input_fire(bot, text)
    return send_target_name_dismiss_waiting(bot, text)
end

local function change_role(bot, target_name, role)
    progress(bot, string.format("change_role target=%s role=%s", target_name, tostring(role)))
    local packet, err = open_category(bot, CAT_OPERATION)
    if packet == nil then
        return nil, err
    end
    packet = select_list_expect_input(bot, OP_ROLE)
    if packet == nil then
        return nil, "role name input missing"
    end
    packet = bot:request_dialog(
        protocol.dialog("INPUT", 0, target_name, 0, 0, "", "NEXT"),
        function(p)
            return p.type == "input"
        end)
    if packet == nil then
        return nil, "role second prompt missing"
    end
    packet = send_input_expect(bot, tostring(role), "normal")
    if packet == nil then
        return nil, "role result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function restore_bot_home(bot, index)
    bot:map_move(HOME_MAP, home_x(index), HOME_Y)
    bot:direction("BOTTOM")
end

local function diplomacy_scenario(label, action_index, requester_idx, target_idx, success_msg)
    return {
        parallel = {
            [requester_idx] = {
                function(ctx)
                    local requester = ctx:bot(requester_idx)
                    local target    = ctx:bot(target_idx)
                    progress(requester, label .. " (requester)")
                    g_invite_ok = false
                    g_inviter_msg = nil
                    g_invitee_msg = nil
                    ctx:sleep(LISTENER_ARM_MS)

                    local ok, err = diplomacy_start(requester, action_index)
                    if ok == false then
                        progress(requester, "FAILED: " .. tostring(err))
                        return false
                    end
                    local packet = send_target_name_expect_normal(requester, target:name())
                    g_inviter_msg = packet and packet.message or nil
                    if packet ~= nil then
                        dismiss_normal(requester)
                    end
                    local pass = g_inviter_msg ~= nil and g_inviter_msg:find(success_msg, 1, true) ~= nil
                    if pass == false then
                        progress(requester, "FAILED: msg=" .. tostring(g_inviter_msg))
                    end
                    g_invite_ok = pass
                    return pass
                end,
            },
            [target_idx] = {
                function(ctx)
                    local target = ctx:bot(target_idx)
                    progress(target, label .. " (target)")
                    local packet = wait_menu(target)
                    if packet == nil then
                        progress(target, "FAILED: menu missing")
                        return false
                    end
                    packet = reply_menu_expect_normal(target, CONFIRM_YES)
                    g_invitee_msg = packet and packet.message or nil
                    if packet ~= nil then
                        dismiss_normal(target)
                    end
                    local pass = g_invitee_msg ~= nil and g_invitee_msg:find(success_msg, 1, true) ~= nil
                    if pass == false then
                        progress(target, "FAILED: msg=" .. tostring(g_invitee_msg))
                    end
                    return pass
                end,
            },
        },
    }
end

test_suite {
    name      = "Clan Test",
    bot_count = 4,

    on_initialize = function(ctx)
        log("debug", "Clan: test initialized")
        lib.formation.arrange_in_line(ctx)
        g_npc = nil
        setup_npc(ctx:bot(0))
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            ctx:bot(i):chat("/엔피씨제거")
        end
        g_npc = nil
        ctx:sleep(500)
    end,

    scenarios = {
        -- 1) Create + duplicate name
        function(ctx)
            local master = ctx:bot(0)
            local other  = ctx:bot(1)
            progress(master, "S1: CREATE")

            local msg, err = create_clan(master, master:name())
            if msg == nil or msg:find(MSG_CREATE_OK, 1, true) == nil then
                progress(master, "FAILED: create " .. tostring(msg or err))
                return false
            end

            progress(other, "S1: CREATE DUP")
            msg, err = create_clan(other, master:name())
            if msg == nil or msg:find(MSG_CREATE_DUP, 1, true) == nil then
                progress(other, "FAILED: dup " .. tostring(msg or err))
                return false
            end

            progress(master, "S1 PASSED")
            return true
        end,

        -- 2) Title short / same / ok
        function(ctx)
            local master = ctx:bot(0)
            progress(master, "S2: TITLE")

            -- Use 1-byte title: UTF-8 Hangul ("가") is 3 bytes and bypasses local length < 2 check.
            local msg, err = change_title(master, "a")
            if is_title_too_short(msg) == false then
                progress(master, "FAILED: short " .. tostring(msg or err))
                return false
            end

            local title = master:name() .. "타이틀"
            msg, err = change_title(master, title)
            if msg == nil or msg:find(MSG_TITLE_OK, 1, true) == nil then
                progress(master, "FAILED: title ok " .. tostring(msg or err))
                return false
            end

            msg, err = change_title(master, title)
            if msg == nil or msg:find(MSG_TITLE_SAME, 1, true) == nil then
                progress(master, "FAILED: same " .. tostring(msg or err))
                return false
            end

            progress(master, "S2 PASSED")
            return true
        end,

        -- 3) Invite not near
        function(ctx)
            local master = ctx:bot(0)
            local spare  = ctx:bot(3)
            progress(master, "S3: INVITE NOT NEAR")

            spare:map_move(OFF_MAP, 1, 1)
            ctx:sleep(500)

            local ok, err = invite_start(master, spare:name())
            if ok == false then
                restore_bot_home(spare, 3)
                progress(master, "FAILED: " .. tostring(err))
                return false
            end
            local packet = send_target_name_expect_normal(master, spare:name())
            if packet == nil or message_contains(packet, MSG_NOT_NEAR) == false then
                restore_bot_home(spare, 3)
                progress(master, "FAILED: not-near msg=" .. tostring(packet and packet.message))
                return false
            end
            dismiss_normal(master)
            restore_bot_home(spare, 3)
            ctx:sleep(300)

            progress(master, "S3 PASSED")
            return true
        end,

        -- 4) Invite reject (mini-parallel)
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local master = ctx:bot(0)
                        local other  = ctx:bot(1)
                        progress(master, "S4: INVITE REJECT (inviter)")
                        ctx:sleep(LISTENER_ARM_MS)

                        local ok, err = invite_start(master, other:name())
                        if ok == false then
                            g_invite_err = err
                            return false
                        end
                        local packet = send_target_name_expect_normal(master, other:name())
                        g_inviter_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(master)
                        end
                        local pass = g_inviter_msg ~= nil
                            and g_inviter_msg:find(MSG_REJECTED, 1, true) ~= nil
                        if pass == false then
                            progress(master, "FAILED: reject msg=" .. tostring(g_inviter_msg))
                        end
                        g_invite_ok = pass
                        return pass
                    end,
                },
                [1] = {
                    function(ctx)
                        local other = ctx:bot(1)
                        progress(other, "S4: INVITE REJECT (invitee)")
                        local packet = wait_menu(other)
                        if packet == nil then
                            progress(other, "FAILED: menu missing")
                            return false
                        end
                        -- Decline: invitee gets no follow-up dialog
                        other:send(protocol.dialog("MENU", 0, "", CONFIRM_NO, 0, "", "NEXT"))
                        return true
                    end,
                },
            },
        },

        function(ctx)
            if g_invite_ok ~= true then
                progress(ctx:bot(0), "FAILED: S4 reject step")
                return false
            end
            progress(ctx:bot(0), "S4 PASSED")
            return true
        end,

        -- 5) Invite accept (mini-parallel)
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local master = ctx:bot(0)
                        local other  = ctx:bot(1)
                        progress(master, "S5: INVITE ACCEPT (inviter)")
                        g_invite_ok = false
                        g_inviter_msg = nil
                        g_invitee_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)

                        local ok, err = invite_start(master, other:name())
                        if ok == false then
                            progress(master, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = send_target_name_expect_normal(master, other:name())
                        g_inviter_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(master)
                        end
                        local pass = g_inviter_msg ~= nil
                            and g_inviter_msg:find(MSG_ACCEPTED, 1, true) ~= nil
                        if pass == false then
                            progress(master, "FAILED: accept msg=" .. tostring(g_inviter_msg))
                        end
                        g_invite_ok = pass
                        return pass
                    end,
                },
                [1] = {
                    function(ctx)
                        local other = ctx:bot(1)
                        progress(other, "S5: INVITE ACCEPT (invitee)")
                        local packet = wait_menu(other)
                        if packet == nil then
                            progress(other, "FAILED: menu missing")
                            return false
                        end
                        packet = reply_menu_expect_normal(other, CONFIRM_YES)
                        g_invitee_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(other)
                        end
                        local pass = g_invitee_msg ~= nil
                            and g_invitee_msg:find(MSG_JOINED, 1, true) ~= nil
                        if pass == false then
                            progress(other, "FAILED: joined msg=" .. tostring(g_invitee_msg))
                        end
                        return pass
                    end,
                },
            },
        },

        function(ctx)
            if g_invite_ok ~= true then
                progress(ctx:bot(0), "FAILED: S5 accept inviter")
                return false
            end
            if g_invitee_msg == nil or g_invitee_msg:find(MSG_JOINED, 1, true) == nil then
                progress(ctx:bot(1), "FAILED: S5 accept invitee")
                return false
            end
            progress(ctx:bot(0), "S5 PASSED")
            return true
        end,

        -- 6) Invite already in clan
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local master = ctx:bot(0)
                        local other  = ctx:bot(1)
                        progress(master, "S6: INVITE ALREADY (inviter)")
                        g_invitee_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)

                        local ok, err = invite_start(master, other:name())
                        if ok == false then
                            progress(master, "FAILED: " .. tostring(err))
                            return false
                        end
                        if send_input_fire(master, other:name()) == false then
                            progress(master, "FAILED: waiting dialog missing")
                            return false
                        end
                        g_inviter_msg = nil
                        return true
                    end,
                },
                [1] = {
                    function(ctx)
                        local other = ctx:bot(1)
                        progress(other, "S6: INVITE ALREADY (invitee)")
                        local packet = wait_menu(other)
                        if packet == nil then
                            progress(other, "FAILED: menu missing")
                            return false
                        end
                        packet = reply_menu_expect_normal(other, CONFIRM_YES)
                        g_invitee_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(other)
                        end
                        local pass = g_invitee_msg ~= nil
                            and g_invitee_msg:find(MSG_ALREADY, 1, true) ~= nil
                        if pass == false then
                            progress(other, "FAILED: already msg=" .. tostring(g_invitee_msg))
                        end
                        return pass
                    end,
                },
            },
        },

        function(ctx)
            if g_invitee_msg == nil or g_invitee_msg:find(MSG_ALREADY, 1, true) == nil then
                progress(ctx:bot(1), "FAILED: S6 already-in")
                return false
            end
            progress(ctx:bot(0), "S6 PASSED")
            return true
        end,

        -- 7) Mate invite privilege fail
        {
            parallel = {
                [1] = {
                    function(ctx)
                        local mate     = ctx:bot(1)
                        local outsider = ctx:bot(2)
                        progress(mate, "S7: MATE INVITE FAIL (inviter)")
                        g_invitee_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)

                        local ok, err = invite_start(mate, outsider:name())
                        if ok == false then
                            progress(mate, "FAILED: " .. tostring(err))
                            return false
                        end
                        if send_input_fire(mate, outsider:name()) == false then
                            progress(mate, "FAILED: waiting dialog missing")
                            return false
                        end
                        return true
                    end,
                },
                [2] = {
                    function(ctx)
                        local outsider = ctx:bot(2)
                        progress(outsider, "S7: MATE INVITE FAIL (invitee)")
                        local packet = wait_menu(outsider)
                        if packet == nil then
                            progress(outsider, "FAILED: menu missing")
                            return false
                        end
                        packet = reply_menu_expect_normal(outsider, CONFIRM_YES)
                        g_invitee_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(outsider)
                        end
                        local pass = is_privilege_error(g_invitee_msg)
                        if pass == false then
                            progress(outsider, "FAILED: priv msg=" .. tostring(g_invitee_msg))
                        end
                        return pass
                    end,
                },
            },
        },

        function(ctx)
            if is_privilege_error(g_invitee_msg) == false then
                progress(ctx:bot(2), "FAILED: S7 privilege")
                return false
            end
            progress(ctx:bot(0), "S7 PASSED")
            return true
        end,

        -- 8) Promote deputy, deputy invite, role/title privilege
        function(ctx)
            local master   = ctx:bot(0)
            local deputy   = ctx:bot(1)
            local outsider = ctx:bot(2)
            progress(master, "S8: PROMOTE + PRIVILEGE")

            local msg, err = change_role(master, deputy:name(), ROLE_DEPUTY)
            if msg == nil or msg:find(MSG_ROLE_OK, 1, true) == nil then
                progress(master, "FAILED: promote " .. tostring(msg or err))
                return false
            end

            -- Deputy invites outsider (needs parallel)
            g_invite_ok = false
            g_invitee_msg = nil
            g_inviter_msg = nil
            return true
        end,

        {
            parallel = {
                [1] = {
                    function(ctx)
                        local deputy   = ctx:bot(1)
                        local outsider = ctx:bot(2)
                        progress(deputy, "S8b: DEPUTY INVITE (inviter)")
                        ctx:sleep(LISTENER_ARM_MS)
                        local ok, err = invite_start(deputy, outsider:name())
                        if ok == false then
                            progress(deputy, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = send_target_name_expect_normal(deputy, outsider:name())
                        g_inviter_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(deputy)
                        end
                        local pass = g_inviter_msg ~= nil
                            and g_inviter_msg:find(MSG_ACCEPTED, 1, true) ~= nil
                        g_invite_ok = pass
                        if pass == false then
                            progress(deputy, "FAILED: msg=" .. tostring(g_inviter_msg))
                        end
                        return pass
                    end,
                },
                [2] = {
                    function(ctx)
                        local outsider = ctx:bot(2)
                        progress(outsider, "S8b: DEPUTY INVITE (invitee)")
                        local packet = wait_menu(outsider)
                        if packet == nil then
                            return false
                        end
                        packet = reply_menu_expect_normal(outsider, CONFIRM_YES)
                        g_invitee_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(outsider)
                        end
                        return g_invitee_msg ~= nil
                            and g_invitee_msg:find(MSG_JOINED, 1, true) ~= nil
                    end,
                },
            },
        },

        function(ctx)
            local master   = ctx:bot(0)
            local deputy   = ctx:bot(1)
            local outsider = ctx:bot(2)

            if g_invite_ok ~= true then
                progress(deputy, "FAILED: S8b deputy invite")
                return false
            end

            local msg, err = change_role(deputy, outsider:name(), ROLE_MASTER)
            if msg == nil or is_privilege_error(msg) == false then
                progress(deputy, "FAILED: role-to-master " .. tostring(msg or err))
                return false
            end

            msg, err = change_title(deputy, "DeputyTitle")
            if msg == nil or msg:find(MSG_TITLE_PRIV, 1, true) == nil then
                progress(deputy, "FAILED: deputy title " .. tostring(msg or err))
                return false
            end

            msg, err = change_title(outsider, "MateTitle")
            if msg == nil or msg:find(MSG_TITLE_PRIV, 1, true) == nil then
                progress(outsider, "FAILED: mate title " .. tostring(msg or err))
                return false
            end

            msg, err = change_title(master, "MasterTitle")
            if msg == nil or msg:find(MSG_TITLE_OK, 1, true) == nil then
                progress(master, "FAILED: master title " .. tostring(msg or err))
                return false
            end

            progress(master, "S8 PASSED")
            return true
        end,

        -- 9) Leave / kick / destroy exceptions + teardown
        function(ctx)
            local master   = ctx:bot(0)
            local deputy   = ctx:bot(1)
            local outsider = ctx:bot(2)
            progress(master, "S9: LEAVE/KICK/DESTROY")

            local msg, err = leave_clan(master)
            if msg == nil or msg:find(MSG_LEAVE_MASTER, 1, true) == nil then
                progress(master, "FAILED: master leave " .. tostring(msg or err))
                return false
            end

            msg, err = kick_member(outsider, deputy:name())
            if msg == nil or is_privilege_error(msg) == false then
                progress(outsider, "FAILED: mate kick " .. tostring(msg or err))
                return false
            end

            msg, err = destroy_clan(master)
            if msg == nil or msg:find(MSG_DESTROY_MEMBERS, 1, true) == nil then
                progress(master, "FAILED: destroy members " .. tostring(msg or err))
                return false
            end

            msg, err = leave_clan(outsider)
            if msg == nil or msg:find(MSG_LEAVE_OK, 1, true) == nil then
                progress(outsider, "FAILED: leave " .. tostring(msg or err))
                return false
            end

            msg, err = kick_member(master, deputy:name())
            if msg == nil or msg:find(MSG_KICK_OK, 1, true) == nil then
                progress(master, "FAILED: kick " .. tostring(msg or err))
                return false
            end

            msg, err = destroy_clan(master)
            if msg == nil or msg:find(MSG_DESTROY_OK, 1, true) == nil then
                progress(master, "FAILED: destroy " .. tostring(msg or err))
                return false
            end

            progress(master, "S9 PASSED")
            return true
        end,

        -- 10) Diplomacy setup: two fresh single-member clans (bot0, bot2)
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(2)
            progress(a, "S10: DIPLOMACY SETUP")

            local msg, err = create_clan(a, a:name())
            if msg == nil or msg:find(MSG_CREATE_OK, 1, true) == nil then
                progress(a, "FAILED: create a " .. tostring(msg or err))
                return false
            end

            msg, err = create_clan(b, b:name())
            if msg == nil or msg:find(MSG_CREATE_OK, 1, true) == nil then
                progress(b, "FAILED: create b " .. tostring(msg or err))
                return false
            end

            progress(a, "S10 PASSED")
            return true
        end,

        -- 11) Alliance request + accept, then verify 동맹 현황
        diplomacy_scenario("S11: ALLY", DIP_ALLY, 0, 2, MSG_ALLY_OK),

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(2)
            if g_invite_ok ~= true then
                progress(a, "FAILED: S11 ally requester")
                return false
            end
            if g_invitee_msg == nil or g_invitee_msg:find(MSG_ALLY_OK, 1, true) == nil then
                progress(b, "FAILED: S11 ally target")
                return false
            end

            local msg, err = fetch_info(a, INFO_ALLY)
            if msg == nil or msg:find(b:name(), 1, true) == nil then
                progress(a, "FAILED: ally info " .. tostring(msg or err))
                return false
            end

            progress(a, "S11 PASSED")
            return true
        end,

        -- 12) Break alliance, then verify 동맹 현황 shows none
        diplomacy_scenario("S12: UNALLY", DIP_UNALLY, 0, 2, MSG_UNALLY_OK),

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(2)
            if g_invite_ok ~= true then
                progress(a, "FAILED: S12 unally requester")
                return false
            end
            if g_invitee_msg == nil or g_invitee_msg:find(MSG_UNALLY_OK, 1, true) == nil then
                progress(b, "FAILED: S12 unally target")
                return false
            end

            local msg, err = fetch_info(a, INFO_ALLY)
            if msg == nil or msg:find(MSG_NONE, 1, true) == nil then
                progress(a, "FAILED: ally info after break " .. tostring(msg or err))
                return false
            end

            progress(a, "S12 PASSED")
            return true
        end,

        -- 13) Declare enemy + accept, then verify 전쟁 현황
        diplomacy_scenario("S13: ENEMY", DIP_ENEMY, 0, 2, MSG_ENEMY_OK),

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(2)
            if g_invite_ok ~= true then
                progress(a, "FAILED: S13 enemy requester")
                return false
            end
            if g_invitee_msg == nil or g_invitee_msg:find(MSG_ENEMY_OK, 1, true) == nil then
                progress(b, "FAILED: S13 enemy target")
                return false
            end

            local msg, err = fetch_info(a, INFO_ENEMY)
            if msg == nil or msg:find(b:name(), 1, true) == nil then
                progress(a, "FAILED: enemy info " .. tostring(msg or err))
                return false
            end

            progress(a, "S13 PASSED")
            return true
        end,

        -- 14) End enmity, verify 전쟁 현황 shows none, then teardown both clans
        diplomacy_scenario("S14: UNENEMY", DIP_UNENEMY, 0, 2, MSG_UNENEMY_OK),

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(2)
            if g_invite_ok ~= true then
                progress(a, "FAILED: S14 unenemy requester")
                return false
            end
            if g_invitee_msg == nil or g_invitee_msg:find(MSG_UNENEMY_OK, 1, true) == nil then
                progress(b, "FAILED: S14 unenemy target")
                return false
            end

            local msg, err = fetch_info(a, INFO_ENEMY)
            if msg == nil or msg:find(MSG_NONE, 1, true) == nil then
                progress(a, "FAILED: enemy info after end " .. tostring(msg or err))
                return false
            end

            msg, err = destroy_clan(a)
            if msg == nil or msg:find(MSG_DESTROY_OK, 1, true) == nil then
                progress(a, "FAILED: cleanup destroy a " .. tostring(msg or err))
                return false
            end

            msg, err = destroy_clan(b)
            if msg == nil or msg:find(MSG_DESTROY_OK, 1, true) == nil then
                progress(b, "FAILED: cleanup destroy b " .. tostring(msg or err))
                return false
            end

            progress(a, "S14 PASSED")
            return true
        end,
    },
}
