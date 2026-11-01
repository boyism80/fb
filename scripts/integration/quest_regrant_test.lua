local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_GREATWALL     = 199
local QUEST_GHOST         = 206
local QUEST_PYOSIN        = 212
local QUEST_BEGINNER_PATH = 238

local TALISMAN = "귀신퇴치부적"
local COVER    = "대장군의일기겉표지"

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, press, next_dialog, select_option = qd.click, qd.press, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "quest_regrant: " .. string.format(fmt, ...))
    return false
end

local function accept_ghost(bot)
    local dlg = click(bot, "안도", contains("이글거리는 기운"))
    dlg = dlg and next_dialog(bot, contains("지나가던 스님께"))
    dlg = dlg and next_dialog(bot, is_list)
    dlg = dlg and select_option(bot, dlg, "제가 좀 도와 드리죠.", contains("그래 주겠는가? 고맙네."))
    return dlg and next_dialog(bot, contains("이 부적을 가지고"))
end

local function pyosin_step12(bot, final)
    local dlg = click(bot, "표신궁역사가", is_list)
    dlg = dlg and select_option(bot, dlg, "표신궁에 관한 이야기를 한다.", contains("여섯번째 일기"))
    dlg = dlg and next_dialog(bot, contains("대장군이 천하통일"))
    dlg = dlg and next_dialog(bot, contains("......"))
    dlg = dlg and next_dialog(bot, contains("......혹시!?"))
    dlg = dlg and next_dialog(bot, contains("큰일이오"))
    return dlg and next_dialog(bot, contains(final))
end

local function pyosin_story(bot, expect)
    local dlg = click(bot, "표신궁역사가", is_list)
    return dlg and select_option(bot, dlg, "표신궁에 관한 이야기를 한다.", expect)
end

local function beginner_path(bot, final)
    local dlg = click(bot, "신규낙랑", contains("초보자방의 마지막까지"))
    dlg = dlg and next_dialog(bot, contains("드 넓은 바람의 나라"))
    dlg = dlg and next_dialog(bot, contains("수천/수만"))
    dlg = dlg and next_dialog(bot, contains("노란비서 20개"))
    dlg = dlg and next_dialog(bot, contains("[노란비서]라는 아이템"))
    dlg = dlg and next_dialog(bot, contains("1회용"))
    return dlg and next_dialog(bot, contains(final))
end

test_suite {
    name      = "Quest Regrant Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "reset_greatwall", QUEST_GREATWALL, "none")
        check(bot, "set_quest", "reset_ghost", QUEST_GHOST, "none")
        check(bot, "set_quest", "reset_pyosin", QUEST_PYOSIN, "none")
        check(bot, "set_quest", "reset_beginner", QUEST_BEGINNER_PATH, "none")
        check(bot, "set_promotion", "reset_promotion", "0")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "greatwall_init", QUEST_GREATWALL, "6") == false then
                return fail("could not prepare QUEST_GREATWALL step 6")
            end
            if click(bot, "서천좌상", contains("저번엔 정말 고마웠네")) == nil then
                return fail("서천좌상 thank-you dialog missing")
            end
            if check(bot, "verify_quest", "greatwall_done", QUEST_GREATWALL, "6:done") == false then
                return fail("the great wall quest stayed open after the last reward")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "ghost_init", QUEST_GHOST, "none") == false then
                return fail("could not reset QUEST_GHOST")
            end

            bot:fill_inventory("목도")
            if accept_ghost(bot) == nil or next_dialog(bot, contains("소지품이 가득 차서 귀신퇴치부적")) == nil then
                return fail("안도 full inventory flow broke")
            end
            if check(bot, "verify_quest", "ghost_full", QUEST_GHOST, "none") == false then
                return fail("안도 started the quest without giving the talismans")
            end

            bot:clear_inventory()
            if accept_ghost(bot) == nil then
                return fail("안도 does not offer the quest again after a full-inventory failure")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "ghost_started", QUEST_GHOST, "1") == false
                or check(bot, "verify_items", "ghost_given", TALISMAN .. "=5") == false then
                return fail("안도 did not give the talismans and start the quest")
            end

            if check(bot, "set_quest_progress", "ghost_buried", QUEST_GHOST, "2") == false
                or check(bot, "remove_items", "ghost_used", TALISMAN, "2") == false then
                return fail("could not bury two talismans")
            end
            if click(bot, "안도", contains("아직 부적을 다 묻지 못한 것 같군")) == nil then
                return fail("안도 handed out talismans although enough are left")
            end
            qd.close(bot)

            if check(bot, "remove_items", "ghost_lost", TALISMAN, "2") == false then
                return fail("could not lose two talismans")
            end
            if click(bot, "안도", contains("부적이 모자란 모양이군")) == nil then
                return fail("안도 did not top up the lost talismans")
            end
            if check(bot, "verify_items", "ghost_topped", TALISMAN .. "=3") == false then
                return fail("안도 did not give exactly the missing talismans")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_promotion", "pyosin_promotion", "1") == false
                or check(bot, "set_quest", "pyosin_init", QUEST_PYOSIN, "12") == false then
                return fail("could not prepare QUEST_PYOSIN step 12")
            end

            bot:fill_inventory("목도")
            if pyosin_step12(bot, "소지품이 가득 차서 대장군의일기겉표지") == nil then
                return fail("표신궁역사가 full inventory flow broke")
            end
            if check(bot, "verify_quest", "pyosin_full", QUEST_PYOSIN, "12") == false then
                return fail("표신궁역사가 moved to step 13 without giving the cover")
            end

            bot:clear_inventory()
            if pyosin_step12(bot, "여기 책 겉표지도 있으니") == nil then
                return fail("표신궁역사가 does not give the cover after a full-inventory failure")
            end
            if check(bot, "verify_quest", "pyosin_step13", QUEST_PYOSIN, "13") == false
                or check(bot, "verify_items", "pyosin_cover", COVER .. "=1") == false then
                return fail("표신궁역사가 did not give the cover and move to step 13")
            end

            qd.close(bot)
            if check(bot, "remove_items", "pyosin_cover_lost", COVER, "1") == false then
                return fail("could not lose the cover")
            end
            if pyosin_story(bot, contains("여기 책 겉표지도 있으니")) == nil then
                return fail("표신궁역사가 did not give the missing cover at step 13")
            end
            if check(bot, "verify_items", "pyosin_cover_again", COVER .. "=1") == false then
                return fail("the cover was not given again")
            end

            qd.close(bot)
            if pyosin_story(bot, contains("어서 조합해보시게")) == nil then
                return fail("표신궁역사가 gave another cover while one is held")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "beginner_init", QUEST_BEGINNER_PATH, "none") == false then
                return fail("could not reset QUEST_BEGINNER_PATH")
            end

            bot:fill_inventory("목도")
            if beginner_path(bot, "소지품이 가득 차서 노란비서를 줄 수 없습니다") == nil then
                return fail("신규낙랑 full inventory flow broke")
            end
            if check(bot, "verify_quest", "beginner_full", QUEST_BEGINNER_PATH, "none") == false then
                return fail("신규낙랑 recorded the last step without giving 노란비서")
            end

            bot:clear_inventory()
            if beginner_path(bot, "<노란비서>을 얻다") == nil then
                return fail("신규낙랑 does not give 노란비서 after a full-inventory failure")
            end
            if check(bot, "verify_quest", "beginner_done", QUEST_BEGINNER_PATH, "7") == false
                or check(bot, "verify_items", "beginner_items", "노란비서=20") == false then
                return fail("신규낙랑 did not give 노란비서 and record the last step")
            end
            return true
        end,
    },
}
