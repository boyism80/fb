local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local QUEST_RABBIT_LIVER = 214
local MOB_NAME           = "토깽이"
local ITEM_RABBIT_LIVER  = "토끼의간"
local SPELL_HELLFIRE     = "헬파이어"
local DEFAULT_INTERVAL   = 100
local DIE_WAIT_MS        = 5000

local function expect_list(message_substr)
    return function(packet)
        if packet.type ~= "list" then
            return false
        end
        if message_substr == nil then
            return true
        end
        return packet.message ~= nil and packet.message:find(message_substr, 1, true) ~= nil
    end
end

local function expect_normal(message_substr)
    return function(packet)
        if packet.type ~= "normal" then
            return false
        end
        if message_substr == nil then
            return true
        end
        return packet.message ~= nil and packet.message:find(message_substr, 1, true) ~= nil
    end
end

local function find_list_index(list_packet, option_text)
    local lists = list_packet and list_packet.list_lists or nil
    if lists == nil then
        return nil
    end
    for i, text in ipairs(lists) do
        if text == option_text then
            return i - 1
        end
    end
    return nil
end

-- Select list/menu option by option text using list_lists from the received packet.
local function select_option(bot, list_packet, option_text, expect_fn)
    local index = find_list_index(list_packet, option_text)
    if index == nil then
        log("fatal", string.format("rabbit quest: list option not found: %s", option_text))
        return nil
    end
    bot:chat(option_text)
    return bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        expect_fn)
end

test_suite {
    name      = "Rabbit Quest Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Rabbit quest test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:chat("/몬스터범위제거 5")
            bot:chat("/아이템삭제")
            bot:chat(string.format("/퀘스트제거 %d", QUEST_RABBIT_LIVER))
        end
        ctx:sleep(DEFAULT_INTERVAL)
    end,

    scenarios = {
        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            local died = false

            log("debug", "rabbit quest: setup bots")
            bot1:setup_bot_stats(100000, 100000)
            bot2:setup_bot_stats(100000, 100000)

            local hell1 = bot1:learn_spell(SPELL_HELLFIRE)
            local hell2 = bot2:learn_spell(SPELL_HELLFIRE)
            if hell1 < 0 or hell2 < 0 then
                log("fatal", "rabbit quest: failed to learn 헬파이어")
                return false
            end

            bot1:chat("/아이템삭제")
            bot1:chat(string.format("/퀘스트설정 %d 2 0", QUEST_RABBIT_LIVER))
            ctx:sleep(DEFAULT_INTERVAL)

            local pos = bot1:position()
            local mob = bot1:spawn_monster(MOB_NAME, pos[1], pos[2] + 1)
            if mob == nil or mob.oid == 0 then
                log("fatal", "rabbit quest: failed to spawn 토깽이")
                return false
            end
            local mob_oid = mob.oid
            log("debug", string.format("rabbit quest: spawned 토깽이 oid=%d", mob_oid))

            ctx:hook("die", function(_, _, packet)
                if packet.oid == mob_oid then
                    died = true
                    log("debug", "rabbit quest: die packet received")
                end
            end)

            -- Bot1 kills 토깽이 -> ON_MOB_KILL opens first list dialog; mob stays invincible.
            log("debug", "rabbit quest: bot1 hellfire kill (expect list dialog)")
            local dlg = bot1:request_dialog_ext(
                protocol.spell_cast("TARGET", hell1, "", mob_oid, mob.position),
                expect_list("여보시오"))
            if dlg == nil then
                log("fatal", "rabbit quest: missing first dialog after kill")
                ctx:unhook("die")
                return false
            end
            if died then
                log("fatal", "rabbit quest: mob died before dialog flow")
                ctx:unhook("die")
                return false
            end

            -- Bot2 hellfire while invincible — mob must survive.
            log("debug", "rabbit quest: bot2 hellfire while invincible")
            bot2:chat("/마력바꾸기 10000")
            ctx:sleep(DEFAULT_INTERVAL)

            bot2:request(
                resp.update_internal,
                protocol.spell_cast("TARGET", hell2, "", mob_oid, mob.position),
                function(packet)
                    return packet.ch_mp == 0
                end)

            ctx:sleep(DEFAULT_INTERVAL)
            if died then
                log("fatal", "rabbit quest: mob died from bot2 hellfire (expected invincible)")
                ctx:unhook("die")
                return false
            end
            log("debug", "rabbit quest: invincible check passed")

            -- Continue dialog: select by option text via list_lists index.
            log("debug", "rabbit quest: dialog step 1 (간이 필요하다)")
            dlg = select_option(bot1, dlg, "나는 네 간이 필요하다", expect_list("하하하"))
            if dlg == nil then
                ctx:unhook("die")
                return false
            end

            log("debug", "rabbit quest: dialog step 2 (간은 어디에)")
            dlg = select_option(bot1, dlg, "그렇다면 네 간은 지금 어디에 있단 말이냐?", expect_normal("숨겨"))
            if dlg == nil then
                ctx:unhook("die")
                return false
            end

            log("debug", "rabbit quest: dialog step 3 (normal next)")
            dlg = bot1:request_dialog_ext(
                protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"),
                expect_list("어떻소"))
            if dlg == nil then
                log("fatal", "rabbit quest: missing list after normal dialog")
                ctx:unhook("die")
                return false
            end

            log("debug", "rabbit quest: dialog step 4 (속지 않음)")
            dlg = select_option(bot1, dlg, "흥, 내가 속을줄 아느냐?", expect_normal("으으윽"))
            if dlg == nil then
                ctx:unhook("die")
                return false
            end

            log("debug", "rabbit quest: dialog step 5 (close final dialog, expect die)")
            bot1:request(
                resp.die,
                protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"),
                function(packet)
                    return packet.oid == mob_oid
                end,
                DIE_WAIT_MS)

            if died == false then
                log("fatal", "rabbit quest: mob was not removed after dialog")
                ctx:unhook("die")
                return false
            end

            if bot1:has_item_by_name(ITEM_RABBIT_LIVER) == false then
                log("fatal", "rabbit quest: missing 토끼의간 after capture")
                ctx:unhook("die")
                return false
            end

            ctx:unhook("die")
            log("debug", "rabbit quest: scenario completed")
            return true
        end,
    },
}
