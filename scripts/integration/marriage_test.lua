local lib      = require("integration.lib")
local protocol = require("integration.protocol")
local resp     = require("integration.response")

local NPC_NAME   = "주례사"
local ITEM_CALLER = "사랑호출기"
local OFF_MAP    = "가상계"
local HOME_MAP   = "낙랑의방"
local HOME_Y     = 6

local OPT_MARRY    = "결혼을 하려고 왔습니다"
local OPT_DIVORCE  = "이혼을 하려고 왔습니다"
local CONFIRM_YES  = 1
local CONFIRM_NO   = 2

local MIN_LEVEL = 21

local MSG_ALREADY_MARRIED     = "이미 결혼을 하셨습니다."
local MSG_REMARRY_COOLDOWN    = "재혼 대기 시간이 지나지 않았습니다"
local MSG_LEVEL_SELF          = "레벨 21 이상만 결혼할 수 있습니다."
local MSG_NOT_NEAR            = "님은 근처에 없습니다."
local MSG_MARRY_SELF          = "자신과는 결혼할 수 없습니다."
local MSG_SAME_GENDER         = "같은 성별끼리는 결혼할 수 없습니다."
local MSG_TARGET_MARRIED      = "이미 결혼한 상대와는 결혼을 할 수 없습니다."
local MSG_TARGET_REMARRY      = "상대방은 아직 재혼할 수 없습니다."
local MSG_LEVEL_TARGET        = "결혼할 상대의 레벨이 21 이상이어야 합니다."
local MSG_MARRY_OK            = "님과 결혼했습니다."
local MSG_REJECTED            = "님이 거절하셨습니다."
local MSG_DIVORCE_ONLY        = "결혼을 한 사람만이 이혼을 할 수 있습니다."
local MSG_DIVORCE_OK          = "이혼이 완료되었습니다. 7일 후 재혼이 가능합니다."
local MSG_CALLER_NOT_MARRIED  = "결혼하지 않아 사용할 수 없습니다."
local MSG_SPOUSE_PREFIX       = "배우자: "
local MSG_ASKING              = "에게 의사를 묻고 있습니다."

local LISTENER_ARM_MS = 500
local TIME_FORWARD_7D = "7.00:00:00"

local g_npc          = nil
local g_parallel_ok  = false
local g_parallel_msg = nil
local g_partner_msg  = nil

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("marriage_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function is_asking_dialog(msg)
    return msg ~= nil and msg:find(MSG_ASKING, 1, true) ~= nil
end

local function home_x(bot_index)
    return 6 + bot_index
end

local function restore_home(bot, index)
    bot:map_move(HOME_MAP, home_x(index), HOME_Y)
    bot:direction("BOTTOM")
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

local function ensure_npc(bot)
    if g_npc ~= nil then
        return g_npc
    end
    return setup_npc(bot)
end

local function dismiss_normal(bot)
    bot:send(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"))
end

local function click_expect(bot, expect_type)
    local npc = ensure_npc(bot)
    local use_ext = (expect_type == "list" or expect_type == "normal" or expect_type == "input_ext")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    return fn(bot,
        protocol.click(npc.oid),
        function(p)
            return p.type == expect_type
        end)
end

local function select_pursuit(bot, option, expect_type)
    expect_type = expect_type or "pursuit"
    local use_ext = (expect_type == "list" or expect_type == "normal" or expect_type == "input_ext")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    return fn(bot,
        protocol.dialog("PURSUIT", 0, "", 0, 0, option),
        function(p)
            return p.type == expect_type
        end)
end

local function select_pursuit_expect_input(bot, option)
    return bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, option),
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

local function wait_normal(bot)
    return bot:request_dialog_ext(
        protocol.chat(false, "."),
        function(p)
            return p.type == "normal"
        end)
end

-- After name INPUT: waiting immediate dialog then consent result, or exception result only.
local function send_input_expect_consent_result(bot, text)
    local packet = send_input_expect(bot, text, "normal")
    if packet == nil then
        return nil
    end
    if is_asking_dialog(packet.message) then
        dismiss_normal(bot)
        return wait_normal(bot)
    end
    return packet
end

local function open_main_menu(bot)
    local packet = click_expect(bot, "pursuit")
    if packet == nil then
        return nil, "main menu missing"
    end
    return packet, nil
end

-- Select 결혼 and expect an immediate normal error (no input).
local function marry_expect_error(bot)
    local packet, err = open_main_menu(bot)
    if packet == nil then
        return nil, err
    end
    packet = select_pursuit(bot, OPT_MARRY, "normal")
    if packet == nil then
        return nil, "marry error dialog missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

-- Select 결혼 -> input name -> expect normal on requester (exception / reject / accept result).
local function marry_input_expect_normal(bot, target_name)
    local packet, err = open_main_menu(bot)
    if packet == nil then
        return nil, err
    end
    packet = select_pursuit_expect_input(bot, OPT_MARRY)
    if packet == nil then
        return nil, "marry name input missing"
    end
    packet = send_input_expect(bot, target_name, "normal")
    if packet == nil then
        return nil, "marry result missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function marry_start_input(bot)
    local packet, err = open_main_menu(bot)
    if packet == nil then
        return false, err
    end
    packet = select_pursuit_expect_input(bot, OPT_MARRY)
    if packet == nil then
        return false, "marry name input missing"
    end
    return true, nil
end

local function divorce_expect_error(bot)
    local packet, err = open_main_menu(bot)
    if packet == nil then
        return nil, err
    end
    packet = select_pursuit(bot, OPT_DIVORCE, "normal")
    if packet == nil then
        return nil, "divorce error dialog missing"
    end
    local msg = packet.message
    dismiss_normal(bot)
    return msg, nil
end

local function divorce_select(bot)
    local packet, err = open_main_menu(bot)
    if packet == nil then
        return false, err
    end
    -- Spouse receives MENU; requester first gets immediate asking dialog.
    bot:send(protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_DIVORCE))
    return true, nil
end

-- After divorce list select: dismiss asking dialog, then wait for consent result.
local function divorce_wait_result(bot)
    local waiting = wait_normal(bot)
    if waiting == nil or is_asking_dialog(waiting.message) == false then
        return waiting
    end
    dismiss_normal(bot)
    return wait_normal(bot)
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

local function assert_spouse_info(bot, spouse_name)
    local info = bot:request(
        resp.internal_info,
        protocol.self_info(),
        function()
            return true
        end)
    if info == false or info == nil then
        return false, "internal_info missing"
    end
    local gi = info.group_info or ""
    if spouse_name == nil then
        if gi:find(MSG_SPOUSE_PREFIX, 1, true) ~= nil then
            return false, "spouse still in group_info=" .. gi
        end
        return true, nil
    end
    local expect = MSG_SPOUSE_PREFIX .. spouse_name
    if gi:find(expect, 1, true) == nil then
        return false, "missing '" .. expect .. "' in group_info=" .. gi
    end
    return true, nil
end

local function ensure_item(bot, name)
    local slot = bot:item_slot(name)
    if slot ~= 0xFF then
        return slot
    end
    bot:create_item(name, 1)
    slot = bot:item_slot(name)
    return slot
end

local function use_caller_expect_message(bot, text)
    local slot = ensure_item(bot, ITEM_CALLER)
    if slot == 0xFF then
        return nil, "caller item missing"
    end
    local packet = bot:request(
        resp.message,
        protocol.item_active(slot),
        function(p)
            return p.type == "STATE"
                and p.text ~= nil
                and p.text:find(text, 1, true) ~= nil
        end)
    if packet == false or packet == nil then
        return nil, "caller message missing"
    end
    return packet.text, nil
end

local function use_caller_teleport(ctx, bot, spouse)
    local slot = ensure_item(bot, ITEM_CALLER)
    if slot == 0xFF then
        return false, "caller item missing"
    end
    local spouse_map = spouse:map()
    bot:send(protocol.item_active(slot))
    ctx:sleep(1500)
    if bot:map() ~= spouse_map then
        return false, string.format("map mismatch got=%s want=%s", tostring(bot:map()), tostring(spouse_map))
    end
    local ap = bot:position()
    local bp = spouse:position()
    local dx = math.abs(ap[1] - bp[1])
    local dy = math.abs(ap[2] - bp[2])
    if dx > 2 or dy > 2 then
        return false, string.format("not near spouse a=(%d,%d) b=(%d,%d)", ap[1], ap[2], bp[1], bp[2])
    end
    return true, nil
end

local function time_forward_7d(bot)
    bot:chat("/시간가속 " .. TIME_FORWARD_7D)
end

local function time_reset(bot)
    bot:chat("/현재시간초기화")
end

local function prepare_bots(ctx)
    local a = ctx:bot(0)
    local b = ctx:bot(1)
    local c = ctx:bot(2)
    local d = ctx:bot(3)

    a:gender("MALE")
    b:gender("FEMALE")
    c:gender("MALE")
    d:gender("FEMALE")

    a:level(MIN_LEVEL)
    b:level(MIN_LEVEL)
    c:level(MIN_LEVEL)
    d:level(MIN_LEVEL)

    for i = 0, 3 do
        restore_home(ctx:bot(i), i)
    end
end

test_suite {
    name      = "Marriage Test",
    bot_count = 4,

    on_initialize = function(ctx)
        log("debug", "Marriage: test initialized")
        lib.formation.arrange_in_line(ctx)
        prepare_bots(ctx)
        g_npc = nil
        setup_npc(ctx:bot(0))
    end,

    on_scenario_finished = function(ctx)
        -- Keep shared NPC; recreate only if cleared.
    end,

    on_finished = function(ctx)
        time_reset(ctx:bot(0))
        for i = 0, ctx:bot_count() - 1 do
            ctx:bot(i):chat("/엔피씨제거")
        end
        g_npc = nil
        ctx:sleep(300)
    end,

    scenarios = {
        -- L1: caller while unmarried
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "L1: CALLER NOT MARRIED")
            local msg, err = use_caller_expect_message(a, MSG_CALLER_NOT_MARRIED)
            if msg == nil then
                progress(a, "FAILED: " .. tostring(err))
                return false
            end
            progress(a, "L1 PASSED")
            return true
        end,

        -- M1: level too low
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "M1: LEVEL SELF")
            a:level(MIN_LEVEL - 1)
            local msg, err = marry_expect_error(a)
            a:level(MIN_LEVEL)
            if msg == nil or msg:find(MSG_LEVEL_SELF, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "M1 PASSED")
            return true
        end,

        -- M2: target not near
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "M2: NOT NEAR")
            b:map_move(OFF_MAP, 1, 1)
            ctx:sleep(500)
            local msg, err = marry_input_expect_normal(a, b:name())
            restore_home(b, 1)
            ctx:sleep(300)
            if msg == nil or msg:find(MSG_NOT_NEAR, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "M2 PASSED")
            return true
        end,

        -- M3: marry self
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "M3: MARRY SELF")
            local msg, err = marry_input_expect_normal(a, a:name())
            if msg == nil or msg:find(MSG_MARRY_SELF, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "M3 PASSED")
            return true
        end,

        -- M4: same gender
        function(ctx)
            local a = ctx:bot(0)
            local c = ctx:bot(2)
            progress(a, "M4: SAME GENDER")
            local msg, err = marry_input_expect_normal(a, c:name())
            if msg == nil or msg:find(MSG_SAME_GENDER, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "M4 PASSED")
            return true
        end,

        -- M5: target level too low
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "M5: LEVEL TARGET")
            b:level(MIN_LEVEL - 1)
            local msg, err = marry_input_expect_normal(a, b:name())
            b:level(MIN_LEVEL)
            if msg == nil or msg:find(MSG_LEVEL_TARGET, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "M5 PASSED")
            return true
        end,

        -- M6: reject (parallel)
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local a = ctx:bot(0)
                        local b = ctx:bot(1)
                        progress(a, "M6: REJECT (requester)")
                        g_parallel_ok = false
                        g_parallel_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)
                        local ok, err = marry_start_input(a)
                        if ok == false then
                            progress(a, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = send_input_expect_consent_result(a, b:name())
                        g_parallel_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(a)
                        end
                        local pass = g_parallel_msg ~= nil
                            and g_parallel_msg:find(MSG_REJECTED, 1, true) ~= nil
                        if pass == false then
                            progress(a, "FAILED: msg=" .. tostring(g_parallel_msg))
                        end
                        g_parallel_ok = pass
                        return pass
                    end,
                },
                [1] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "M6: REJECT (target)")
                        local packet = wait_menu(b)
                        if packet == nil then
                            progress(b, "FAILED: menu missing")
                            return false
                        end
                        b:send(protocol.dialog("MENU", 0, "", CONFIRM_NO, 0, "", "NEXT"))
                        return true
                    end,
                },
            },
        },

        function(ctx)
            if g_parallel_ok ~= true then
                progress(ctx:bot(0), "FAILED: M6 reject")
                return false
            end
            progress(ctx:bot(0), "M6 PASSED")
            return true
        end,

        -- M7: accept (parallel)
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local a = ctx:bot(0)
                        local b = ctx:bot(1)
                        progress(a, "M7: ACCEPT (requester)")
                        g_parallel_ok = false
                        g_parallel_msg = nil
                        g_partner_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)
                        local ok, err = marry_start_input(a)
                        if ok == false then
                            progress(a, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = send_input_expect_consent_result(a, b:name())
                        g_parallel_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(a)
                        end
                        local pass = g_parallel_msg ~= nil
                            and g_parallel_msg:find(MSG_MARRY_OK, 1, true) ~= nil
                        if pass == false then
                            progress(a, "FAILED: msg=" .. tostring(g_parallel_msg))
                        end
                        g_parallel_ok = pass
                        return pass
                    end,
                },
                [1] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "M7: ACCEPT (target)")
                        local packet = wait_menu(b)
                        if packet == nil then
                            progress(b, "FAILED: menu missing")
                            return false
                        end
                        packet = reply_menu_expect_normal(b, CONFIRM_YES)
                        g_partner_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(b)
                        end
                        local pass = g_partner_msg ~= nil
                            and g_partner_msg:find(MSG_MARRY_OK, 1, true) ~= nil
                        if pass == false then
                            progress(b, "FAILED: msg=" .. tostring(g_partner_msg))
                        end
                        return pass
                    end,
                },
            },
        },

        function(ctx)
            if g_parallel_ok ~= true then
                progress(ctx:bot(0), "FAILED: M7 accept requester")
                return false
            end
            if g_partner_msg == nil or g_partner_msg:find(MSG_MARRY_OK, 1, true) == nil then
                progress(ctx:bot(1), "FAILED: M7 accept target")
                return false
            end
            progress(ctx:bot(0), "M7 PASSED")
            return true
        end,

        -- I1: internal_info spouse after marriage
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "I1: INTERNAL_INFO MARRIED")
            local ok, err = assert_spouse_info(a, b:name())
            if ok == false then
                progress(a, "FAILED: A " .. tostring(err))
                return false
            end
            ok, err = assert_spouse_info(b, a:name())
            if ok == false then
                progress(b, "FAILED: B " .. tostring(err))
                return false
            end
            progress(a, "I1 PASSED")
            return true
        end,

        -- L2: caller teleport to spouse on other map
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "L2: CALLER TELEPORT")
            b:map_move(OFF_MAP, 5, 5)
            ctx:sleep(500)
            local ok, err = use_caller_teleport(ctx, a, b)
            restore_home(a, 0)
            restore_home(b, 1)
            ctx:sleep(300)
            if ok == false then
                progress(a, "FAILED: " .. tostring(err))
                return false
            end
            progress(a, "L2 PASSED")
            return true
        end,

        -- M8: already married (self)
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "M8: ALREADY MARRIED")
            local msg, err = marry_expect_error(a)
            if msg == nil or msg:find(MSG_ALREADY_MARRIED, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "M8 PASSED")
            return true
        end,

        -- M9: target already married
        function(ctx)
            local c = ctx:bot(2)
            local b = ctx:bot(1)
            progress(c, "M9: TARGET MARRIED")
            local msg, err = marry_input_expect_normal(c, b:name())
            if msg == nil or msg:find(MSG_TARGET_MARRIED, 1, true) == nil then
                progress(c, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(c, "M9 PASSED")
            return true
        end,

        -- D1: unmarried divorce
        function(ctx)
            local c = ctx:bot(2)
            progress(c, "D1: DIVORCE UNMARRIED")
            local msg, err = divorce_expect_error(c)
            if msg == nil or msg:find(MSG_DIVORCE_ONLY, 1, true) == nil then
                progress(c, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(c, "D1 PASSED")
            return true
        end,

        -- D2: divorce reject
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local a = ctx:bot(0)
                        progress(a, "D2: DIVORCE REJECT (requester)")
                        g_parallel_ok = false
                        g_parallel_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)
                        local ok, err = divorce_select(a)
                        if ok == false then
                            progress(a, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = divorce_wait_result(a)
                        g_parallel_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(a)
                        end
                        local pass = g_parallel_msg ~= nil
                            and g_parallel_msg:find(MSG_REJECTED, 1, true) ~= nil
                        g_parallel_ok = pass
                        if pass == false then
                            progress(a, "FAILED: msg=" .. tostring(g_parallel_msg))
                        end
                        return pass
                    end,
                },
                [1] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "D2: DIVORCE REJECT (spouse)")
                        local packet = wait_menu(b)
                        if packet == nil then
                            progress(b, "FAILED: menu missing")
                            return false
                        end
                        b:send(protocol.dialog("MENU", 0, "", CONFIRM_NO, 0, "", "NEXT"))
                        return true
                    end,
                },
            },
        },

        function(ctx)
            if g_parallel_ok ~= true then
                progress(ctx:bot(0), "FAILED: D2")
                return false
            end
            progress(ctx:bot(0), "D2 PASSED")
            return true
        end,

        -- D3: divorce accept
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local a = ctx:bot(0)
                        progress(a, "D3: DIVORCE ACCEPT (requester)")
                        g_parallel_ok = false
                        g_parallel_msg = nil
                        g_partner_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)
                        local ok, err = divorce_select(a)
                        if ok == false then
                            progress(a, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = divorce_wait_result(a)
                        g_parallel_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(a)
                        end
                        local pass = g_parallel_msg ~= nil
                            and g_parallel_msg:find(MSG_DIVORCE_OK, 1, true) ~= nil
                        g_parallel_ok = pass
                        if pass == false then
                            progress(a, "FAILED: msg=" .. tostring(g_parallel_msg))
                        end
                        return pass
                    end,
                },
                [1] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "D3: DIVORCE ACCEPT (spouse)")
                        local packet = wait_menu(b)
                        if packet == nil then
                            progress(b, "FAILED: menu missing")
                            return false
                        end
                        packet = reply_menu_expect_normal(b, CONFIRM_YES)
                        g_partner_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(b)
                        end
                        local pass = g_partner_msg ~= nil
                            and g_partner_msg:find(MSG_DIVORCE_OK, 1, true) ~= nil
                        if pass == false then
                            progress(b, "FAILED: msg=" .. tostring(g_partner_msg))
                        end
                        return pass
                    end,
                },
            },
        },

        function(ctx)
            if g_parallel_ok ~= true then
                progress(ctx:bot(0), "FAILED: D3 requester")
                return false
            end
            if g_partner_msg == nil or g_partner_msg:find(MSG_DIVORCE_OK, 1, true) == nil then
                progress(ctx:bot(1), "FAILED: D3 spouse")
                return false
            end
            progress(ctx:bot(0), "D3 PASSED")
            return true
        end,

        -- I2: internal_info cleared after divorce
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "I2: INTERNAL_INFO DIVORCED")
            local ok, err = assert_spouse_info(a, nil)
            if ok == false then
                progress(a, "FAILED: A " .. tostring(err))
                return false
            end
            ok, err = assert_spouse_info(b, nil)
            if ok == false then
                progress(b, "FAILED: B " .. tostring(err))
                return false
            end
            progress(a, "I2 PASSED")
            return true
        end,

        -- L3: caller after divorce
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "L3: CALLER AFTER DIVORCE")
            local msg, err = use_caller_expect_message(a, MSG_CALLER_NOT_MARRIED)
            if msg == nil then
                progress(a, "FAILED: " .. tostring(err))
                return false
            end
            progress(a, "L3 PASSED")
            return true
        end,

        -- R1: remarry cooldown
        function(ctx)
            local a = ctx:bot(0)
            local d = ctx:bot(3)
            progress(a, "R1: REMARRY COOLDOWN")
            local msg, err = marry_expect_error(a)
            if msg == nil or msg:find(MSG_REMARRY_COOLDOWN, 1, true) == nil then
                progress(a, "FAILED: " .. tostring(msg or err))
                return false
            end
            -- Also cover target-side cooldown message path via D attempting to marry A
            progress(d, "R1b: TARGET REMARRY COOLDOWN")
            msg, err = marry_input_expect_normal(d, a:name())
            if msg == nil or msg:find(MSG_TARGET_REMARRY, 1, true) == nil then
                progress(d, "FAILED: " .. tostring(msg or err))
                return false
            end
            progress(a, "R1 PASSED")
            return true
        end,

        -- R2: time forward then remarry success
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local a = ctx:bot(0)
                        local d = ctx:bot(3)
                        progress(a, "R2: REMARRY AFTER TIME (requester)")
                        time_forward_7d(a)
                        ctx:sleep(1000)
                        g_parallel_ok = false
                        g_parallel_msg = nil
                        g_partner_msg = nil
                        ctx:sleep(LISTENER_ARM_MS)
                        local ok, err = marry_start_input(a)
                        if ok == false then
                            progress(a, "FAILED: " .. tostring(err))
                            return false
                        end
                        local packet = send_input_expect_consent_result(a, d:name())
                        g_parallel_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(a)
                        end
                        local pass = g_parallel_msg ~= nil
                            and g_parallel_msg:find(MSG_MARRY_OK, 1, true) ~= nil
                        g_parallel_ok = pass
                        if pass == false then
                            progress(a, "FAILED: msg=" .. tostring(g_parallel_msg))
                        end
                        return pass
                    end,
                },
                [3] = {
                    function(ctx)
                        local d = ctx:bot(3)
                        progress(d, "R2: REMARRY AFTER TIME (target)")
                        local packet = wait_menu(d)
                        if packet == nil then
                            progress(d, "FAILED: menu missing")
                            return false
                        end
                        packet = reply_menu_expect_normal(d, CONFIRM_YES)
                        g_partner_msg = packet and packet.message or nil
                        if packet ~= nil then
                            dismiss_normal(d)
                        end
                        return g_partner_msg ~= nil
                            and g_partner_msg:find(MSG_MARRY_OK, 1, true) ~= nil
                    end,
                },
            },
        },

        function(ctx)
            time_reset(ctx:bot(0))
            ctx:sleep(500)
            if g_parallel_ok ~= true then
                progress(ctx:bot(0), "FAILED: R2 remarry")
                return false
            end
            progress(ctx:bot(0), "R2 PASSED")
            return true
        end,
    },
}
