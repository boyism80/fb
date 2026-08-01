local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local CANNOT_PICKUP_TEXT = "더 이상 가질 수 없습니다."

local function setup_npc(ctx, bot, index, npc_name)
    log("debug", string.format("Chat[%d]: %s", index, "setup_npc: " .. npc_name))
    bot:direction("BOTTOM")
    bot:move("BOTTOM")

    local npc = bot:create_npc(npc_name)
    log("debug", string.format("Chat[%d]: %s", index, "setup_npc: created npc oid=" .. tostring(npc.oid)))

    bot:move("TOP")
    bot:direction("BOTTOM")

    return npc
end

local function test_scenario_1(ctx, index)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1 started"))
    local bot = ctx:bot(index)

    setup_npc(ctx, bot, index, "왈숙네")

    bot:create_item("진호박", 10)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: created 진호박 x10"))

    local base_price     = bot:item_base_price("진호박")
    local expected_money = math.floor(base_price / 2) * 5
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: selling 5 pumpkins (expected money >= " .. tostring(expected_money) .. ")"))

    bot:request(
        resp.update_internal,
        protocol.chat(false, "진호박 5개 판다"),
        function(packet)
            return packet.ch_money >= expected_money
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: sold 5 pumpkins, money=" .. tostring(bot:money())))
    if bot:money() < expected_money then
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 1 FAILED: money after selling 5"))
        return false
    end

    expected_money = math.floor(base_price / 2) * 10
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: selling all pumpkins"))
    bot:request(
        resp.update_internal,
        protocol.chat(false, "진호박 다 판다"),
        function(packet)
            return packet.ch_money >= expected_money
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: sold all pumpkins, money=" .. tostring(bot:money())))
    if bot:money() < expected_money then
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 1 FAILED: money after selling all"))
        return false
    end

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: depositing all money"))
    bot:request(
        resp.update_internal,
        protocol.chat(false, "돈 다 맡아줘"),
        function(packet)
            return packet.ch_money == 0
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: deposited money, money=" .. tostring(bot:money())))
    if bot:money() ~= 0 then
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 1 FAILED: money not zero after deposit"))
        return false
    end

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: withdrawing all money"))
    bot:request(
        resp.update_internal,
        protocol.chat(false, "돈 다 돌려줘"),
        function(packet)
            return packet.ch_money > 0
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: withdrew money, money=" .. tostring(bot:money())))
    if bot:money() <= 0 then
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 1 FAILED: money not restored"))
        return false
    end

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: requesting 동동주"))
    bot:request(
        resp.item_update,
        protocol.chat(false, "동동주 줘"),
        function(packet)
            return string.find(packet.name, "동동주", 1, true) == 1
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: received 동동주"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: depositing 동동주"))
    bot:request(
        resp.item_remove,
        protocol.chat(false, "동동주 맡아줘"),
        function()
            return true
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: deposited 동동주"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: withdrawing 동동주"))
    bot:request(
        resp.item_update,
        protocol.chat(false, "동동주 돌려줘"),
        function(packet)
            return string.find(packet.name, "동동주", 1, true) == 1
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: withdrew 동동주"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: depositing 도토리 (first batch)"))
    bot:create_item("도토리", 201)
    bot:request(
        resp.item_remove,
        protocol.chat(false, "도토리 다 맡아줘"),
        function()
            return true
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: deposited first 도토리 batch"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: depositing 도토리 (second batch)"))
    bot:create_item("도토리", 201)
    bot:request(
        resp.item_remove,
        protocol.chat(false, "도토리 다 맡아줘"),
        function()
            return true
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: deposited second 도토리 batch"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 1: withdrawing 도토리 (expect cannot pickup)"))
    bot:request(
        resp.chat,
        protocol.chat(false, "도토리 다 돌려줘"),
        function(packet)
            return string.find(packet.text, CANNOT_PICKUP_TEXT, 1, true) ~= nil
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 1 completed successfully"))
    return true
end

local function test_scenario_2(ctx, index)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 2 started"))
    local bot = ctx:bot(index)

    local npc = setup_npc(ctx, bot, index, "떡쇠")

    bot:create_item("양첨목봉", 1)
    bot:money(100000)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: created 양첨목봉, money=100000"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: repair with full durability (expect no items)"))
    bot:request(
        resp.chat,
        protocol.chat(false, "전부 고쳐줘"),
        function(packet)
            if packet.oid ~= npc.oid then
                return false
            end
            return string.find(packet.text, "고칠 물건이 없습니다.", 1, true) ~= nil
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: got expected no-repair response"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: setting durability to 1"))
    bot:chat("/내구도 1")
    ctx:sleep(1000)

    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: repair with low durability"))
    bot:request(
        resp.chat,
        protocol.chat(false, "전부 고쳐줘"),
        function(packet)
            if packet.oid ~= npc.oid then
                return false
            end
            return string.find(packet.text, "고치는데 9900전이 들었습니다.", 1, true) ~= nil
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: repair completed"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 2: renaming item"))
    bot:request(
        resp.item_update,
        protocol.chat(false, "양첨목봉 이름을 채승현으로 명명"),
        function(packet)
            return packet.index == 0 and packet.name == "채승현"
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 2 completed successfully"))
    return true
end

local function test_scenario_3(ctx, index)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 3 started"))
    local bot = ctx:bot(index)

    local npc = setup_npc(ctx, bot, index, "좌성황")

    if lib.option.disable_pk_protect(bot) == false then
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 3 FAILED: could not disable PK_PROTECT"))
        return false
    end

    local spell_index = bot:learn_spell("헬파이어")
    bot:set_max_hp_mp(100000, 100000)
    bot:mp(100000)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: learned 헬파이어, spell_index=" .. tostring(spell_index)))

    local pos = bot:position()
    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: casting 헬파이어 on self"))
    bot:request(
        resp.update_internal,
        protocol.spell_cast("TARGET", spell_index, "", bot:oid(), pos),
        function(packet)
            return packet.ch_hp == 0
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: spell cast done, hp=" .. tostring(bot:hp())))

    local revive_attempt = 0
    while true do
        revive_attempt = revive_attempt + 1
        log("debug", string.format("Chat[%d]: %s", index, "scenario 3: revive request attempt " .. tostring(revive_attempt)))
        local packet = bot:request(
            resp.chat,
            protocol.chat(false, "살려주세요"),
            function(p)
                return p.oid == npc.oid
            end)
        log("debug", string.format("Chat[%d]: %s", index, "scenario 3: revive response: " .. tostring(packet.text)))
        if string.find(packet.text, "너의 정성에 감복하여 살려줄터이니 잠시 기다려라....", 1, true) ~= nil then
            break
        end
    end

    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: waiting for NORMAL state (current=" .. tostring(bot:state()) .. ")"))
    while bot:state() ~= "NORMAL" do
        ctx:sleep(100)
    end
    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: state is NORMAL, hp=" .. tostring(bot:hp())))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: thanking NPC"))
    bot:request(
        resp.update_internal,
        protocol.chat(false, "감사합니다"),
        function(packet)
            return packet.ch_hp == bot:base_hp()
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 3: thank you done, hp=" .. tostring(bot:hp()) .. " base_hp=" .. tostring(bot:base_hp())))
    if bot:hp() ~= bot:base_hp() then
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 3 FAILED: hp not restored to base"))
        return false
    end

    log("debug", string.format("Chat[%d]: %s", index, "scenario 3 completed successfully"))
    return true
end

local function test_scenario_4(ctx, index)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 4 started"))
    local bot = ctx:bot(index)

    local npc = setup_npc(ctx, bot, index, "뭉치")
    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: npc oid=" .. tostring(npc.oid)))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: clicking NPC (expect pursuit dialog)"))
    bot:request_dialog(
        protocol.click(npc.oid),
        function(packet)
            return packet.type == "pursuit"
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: click dialog received (pursuit)"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: select 물건 사기 (expect category pursuit)"))
    bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, "물건 사기"),
        function(packet)
            return packet.type == "pursuit"
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: category pursuit done"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: select category (expect item)"))
    bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, "전사용 갑주류"),
        function(packet)
            return packet.type == "item"
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: item dialog done"))

    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: dialog ITEM step (expect normal)"))
    local packet = bot:request_dialog_ext(
        protocol.dialog("ITEM", 0, "", 0, 0, "unknown"),
        function(p)
            return p.type == "normal"
        end)
    log("debug", string.format("Chat[%d]: %s", index, "scenario 4: ITEM step done, message=" .. tostring(packet.message)))

    local ok = packet.message == "This player is a hacker."
    if ok then
        log("debug", string.format("Chat[%d]: %s", index, "scenario 4 completed successfully"))
    else
        log("fatal", string.format("Chat[%d]: %s", index, "scenario 4 FAILED: unexpected message"))
    end
    return ok
end

test_suite {
    name      = "Chat Interaction Test",
    bot_count = 5,

    on_initialize = function(ctx)
        log("debug", "Chat interaction test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_parallel_scenario_finished = function(ctx, id)
        log("debug", "Chat[" .. tostring(id) .. "]: parallel scenario finished, removing NPCs")
        ctx:bot(id):chat("/엔피씨제거")
        ctx:sleep(1000)
    end,

    scenarios = {
        {
            parallel = {
                [0] = { function(ctx) return test_scenario_1(ctx, 0) end },
                [1] = { function(ctx) return test_scenario_2(ctx, 1) end },
                [2] = { function(ctx) return test_scenario_3(ctx, 2) end },
                [3] = { function(ctx) return test_scenario_4(ctx, 3) end },
            },
        },
    },
}
