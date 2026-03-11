
local function all_sub_quests_complete(me)
    local steps = {
        [QUEST_SKULL_NECKLACE_1] = 2,
        [QUEST_SKULL_NECKLACE_2] = 3,
        [QUEST_SKULL_NECKLACE_3] = 3,
        [QUEST_SKULL_NECKLACE_4] = 3,
        [QUEST_SKULL_NECKLACE_5] = 2,
        [QUEST_SKULL_NECKLACE_6] = 2,
        [QUEST_SKULL_NECKLACE_7] = 3,
        [QUEST_SKULL_NECKLACE_8] = 2,
        [QUEST_SKULL_NECKLACE_9] = 3,
    }
    for qid, need_step in pairs(steps) do
        local q = me:quest(qid)
        if q == nil or q:step() ~= need_step then
            return false
        end
    end
    return true
end

function NPC_474(me, npc)
    local main = me:quest(QUEST_SKULL_NECKLACE)
    local main_step = (main and main:step()) or 0

    if main == nil or main_step == 0 then
        if all_sub_quests_complete(me) then
            if main == nil then
                main = me:start_quest(QUEST_SKULL_NECKLACE)
                if main == nil then
                    me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
                    return
                end
            end
            if main then
                main:step(1)
            end
            me:dialog(npc, "이제 자네에게 부탁이 있네. 다시 말을 걸어주게.", false, false)
            return
        end
        me:dialog(npc, "(여느 원숭이 왕보다 나이가 많고, 품위있어 보이는 원숭이는 당신을 훑어보다가 고개를 돌렸다.)", false, false)
        return
    end

    if main_step == 1 then
        local sel, btn = me:list(npc, "원숭이들을 대표하여 감사드리네.", { "이름을 가르쳐주세요.", "이곳의 상황은 좀 어떤가요?", "제가 도와드릴 일은 없을까요?" }, false)
        if btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            me:dialog(npc, "나의 이름은 알아서 뭐하시려고 그러시는가?", false, false)
            return
        end
        if sel == 1 then
            me:dialog(npc, "이 곳의 상황은 본래와 다를바 없으시다.", false, false)
            return
        end
        ::NPC_474_0009::
        sel, btn = me:list(npc, "음? 하하하. 고마운 말씀이시다. 진심이신가?", { "네, 꼭 도와드릴께요.", "아뇨, 그만 둘래요." }, true)
        if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
            me:dialog(npc, "장난을 치는 사람은 싫어한다.", false, false)
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_474_0009
        end
        ::NPC_474_0010::
        local b = me:dialog(npc, "음, 아직도 도와주고 싶다는 마음이 남아있다니, 정말로 보기 드문 의인이로다. 자네의 마음 씀씀이에 경의를 표하겠네.", true, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if b == DIALOG_RESULT.PREV then
            goto NPC_474_0009
        end
        ::NPC_474_0011::
        b = me:dialog(npc, "허지만, 나는 참된 왕으로서 작은 도움을 구할 수는 없네. 자네가 괴로울만큼 어려운 부탁만을 할지도 몰라.", true, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if b == DIALOG_RESULT.PREV then
            goto NPC_474_0010
        end
        ::NPC_474_0012::
        sel, btn = me:list(npc, "그래도 좋은가?", { "네, 괜찮습니다.", "다시 생각해볼래요." }, true)
        if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_474_0011
        end
        ::NPC_474_0013::
        b = me:dialog(npc, "지금처럼 뿔뿔히 흩어진 상황에서는 모일 수가 없으니, 자네가 대신 의견을 좀 모아다주지 않겠나?", true, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if b == DIALOG_RESULT.PREV then
            goto NPC_474_0012
        end
        b = me:dialog(npc, "나에게 알려주지 않겠나? 부탁하겠네. 아마, 모든 왕들을 한번씩은 만나봐야 할게야. 여유있게 다녀오시게.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(2)
        return
    end

    if main_step == 2 then
        me:dialog(npc, "아마, 모든 왕들을 한번씩은 만나봐야 할게야. 여유있게 다녀오시게.", false, false)
        return
    end

    if main_step == 3 then
        local b = me:dialog(npc, "토원왕께서 좋은 의견을 내주셨으시다. 저번에 구한 적이 있으시다니, 좌황활력환을 구해와 주시길 부탁드리겠다.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(4)
        return
    end

    if main_step == 4 then
        me:dialog(npc, "저번에 좌황활력환을 구한적이 있으시다니, 직접 구해주시길 바라신다.", false, false)
        return
    end

    if main_step == 5 then
        ::NPC_474_0020::
        local b = me:dialog(npc, "음? 그럴수가... 우리 원숭이들은 어떻게 되어도 좋다는건가? 인간도 원숭이도 모두 하늘이 만드신 피조물이거늘...", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "어쩔 수 없지. 자, 이 편지를 가져다가 보여주시게. 그래도 아니 될 일이라면, 다른 대책을 강구해보는 수 밖에 없겠지.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        me:mkitem("원숭이의편지", 1)
        main:step(6)
        return
    end

    if main_step == 6 then
        me:dialog(npc, "이 편지를 가져다가 보여주시게.", false, false)
        return
    end

    if main_step == 7 then
        ::NPC_474_0030::
        local b = me:dialog(npc, "음, 약을 어떻게든 공급해주겠다는 약속을 받아냈으니, 어찌되었든 한숨 돌린 셈이군... 다행이야.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "자, 그럼 다른 왕들의 계책도 들어다주게나. 잘 부탁하네.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(8)
        return
    end

    if main_step == 8 then
        me:dialog(npc, "자, 그런 다른 왕들의 계책도 들어다주게나. 잘 부탁하네.", false, false)
        return
    end

    if main_step == 9 then
        ::NPC_474_0040::
        local b = me:dialog(npc, "그래, 역시 문자를 알아야 연구를 할 수 있겠지. 음, 혹시 이 나라 사람들이 배우는 글책을 구할 수 있을까?", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_474_0041::
        b = me:dialog(npc, "자랑은 아니지만, 나는 꽤 머리가 좋아서 사람들의 문자도 쉽게 배울 수 있다네. 책을 좀 구해다주게나.", true, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if b == DIALOG_RESULT.PREV then
            goto NPC_474_0040
        end
        main:step(10)
        return
    end

    if main_step == 10 then
        me:dialog(npc, "문자를 배울 수 있는 책을 좀 구해다주게나.", false, false)
        return
    end

    if main_step == 11 then
        if not me:has_items("창힐독본", 1) then
            me:dialog(npc, "퀘스트 오류입니다.\n사진을 찍어 홈페이지에 문의하세요.", false, false)
            return
        end
        ::NPC_474_0050::
        local b = me:dialog(npc, "수고했네! 큰도움이 될 거야. 정말 고맙네.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "나머지 한 권은 대원왕에게 가져다 주게. 필요한 재료를 만드는데 도움이 될 걸세.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if not me:rmitem("창힐독본", 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
            return
        end
        main:step(12)
        return
    end

    if main_step == 12 then
        me:dialog(npc, "나머지 한 권을 대원왕에게 가져다 주게.", false, false)
        return
    end

    if main_step == 14 then
        if not me:has_items("진원비전탕", 1) then
            me:dialog(npc, "퀘스트 오류입니다.\n사진을 찍어 홈페이지에 문의하세요.", false, false)
            return
        end
        ::NPC_474_0060::
        local b = me:dialog(npc, "참원왕께서 이 걸 가져다달라고 하셨다고?", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "아, 고맙네. 귀중한 목숨이 위험할때 반드시 요긴하게 쓰도록 하겠네. 자네는 정말 대단한 사람이구만...", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if not me:rmitem("진원비전탕", 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
            return
        end
        main:step(15)
        return
    end

    if main_step == 15 then
        ::NPC_474_0070::
        local b = me:dialog(npc, "정말 애쓰는군. 하지만, 아직 다른 왕들의 의견이나 계책을 더 들어보았으면 한다네. 힘들겠지만,", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "좀 더 수고해서 다른 왕들의 의견을 들어줬으면 하네.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(16)
        return
    end

    if main_step == 16 then
        me:dialog(npc, "좀 더 수고해서 다른 왕들의 의견을 들어주었으면 하네.", false, false)
        return
    end

    if main_step == 17 then
        ::NPC_474_0080::
        local b = me:dialog(npc, "음... 자경단이라... 그래 확실히 전염병은 아니지만, 그 무서운 힘에 다치거나하면 더 큰일이긴하지.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "틀린 말은 아닐세. 철도 3개와 사각방패 3개 정도면 일단 작은 자경단 정도는 구성할 수 있을게야.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(18)
        return
    end

    if main_step == 18 then
        me:dialog(npc, "철도 3개와 사각방패 3개를 금원왕에게 찾아다 주시게.", false, false)
        return
    end

    if main_step == 19 then
        ::NPC_474_0090::
        local b = me:dialog(npc, "이로써 우리 원숭이들도 스스로를 지킬 힘을 가지게 되었구만... 다행이야.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "그럼, 다른 두목들의 계책과 의견을 마저 모아다 주시게나. 기다리고 있겠네.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(20)
        return
    end

    if main_step == 20 then
        me:dialog(npc, "다른 두목들의 계책과 의견을 마저 모아다 주시게나.", false, false)
        return
    end

    if main_step == 21 then
        ::NPC_474_0100::
        local b = me:dialog(npc, "그거, 좋은 생각이군. 다만 구워먹는다고해서 정말 안전하느냐는 의심이 남는데...", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "일단 횃불이란걸 나에게 가져다주게. 몸소 실험해보고, 괜찮다 싶으면 돌려 써야하겠어.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(22)
        return
    end

    if main_step == 22 then
        if not me:has_items("횃불", 1) then
            me:dialog(npc, "일단 횃불란걸 나에게 가져다주게.", false, false)
            return
        end
        if not me:rmitem("횃불", 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
            return
        end
        main:step(23)
        ::NPC_474_0110::
        local b = me:dialog(npc, "고맙네, 과연 음식을 구울 수 있을만큼 뜨거워보이는군. 빨리 구워먹는 법을 익혀야겠어.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "더이상 무슨 의견이나 계책이 나올지 의심스럽지만, 마지막으로 한 번만 더 수고해주지 않겠나.", false, true)
        return
    end

    if main_step == 23 then
        me:dialog(npc, "좀 더 수고해서 다른 왕들의 의견을 들어주게.", false, false)
        return
    end

    if main_step == 24 then
        ::NPC_474_0120::
        local b = me:dialog(npc, "음, 그래 마른갈대가 있으면 아이들이 편하게 잘 수 있을지도 모르겠군.. 일단 그 마른갈대란 것을 구해서 우리 왕들에게", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        ::NPC_474_0121::
        b = me:dialog(npc, "하나씩 나누어 주시게나. 일단 그 마른갈대가 무엇인지만 알면, 우리 나름대로 힘내서 구할 수가 있을게야. 일단", true, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if b == DIALOG_RESULT.PREV then
            goto NPC_474_0120
        end
        b = me:dialog(npc, "아이들이라도 편하게 잘 수 있다면 한시름 놓을 수 있을게야... 잘 부탁하네.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(25)
        return
    end

    if main_step == 25 then
        if not me:has_items("마른갈대", 1) then
            me:dialog(npc, "마른갈대를 구해서 왕들에게 하나씩 나누어 주게.", false, false)
            return
        end
        local b = me:dialog(npc, "마른 갈대를 나눠주고 있다고 들었네. 수고하는 모습이 참 보기 좋군. 더 수고해주게.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        if not me:rmitem("마른갈대", 1, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
            return
        end
        main:step(26)
        return
    end

    if main_step == 35 then
        ::NPC_474_0130::
        local b = me:dialog(npc, "고맙네! 정말 고맙네! 이로써 만반의 대책은 갖춘 셈이야! 자네는 우리 원숭이들의 은인일세.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        b = me:dialog(npc, "원숭이들의 왕인 나 참원왕의 가족일세. 자, 이걸 받으시게. 우리 참원왕 대대로 내려오는 해골 목걸이야.", false, true)
        if b == DIALOG_RESULT.QUIT then
            return
        end
        main:step(36)
        me:mkitem("해골목걸이", 1)
        me:push_achievement(63, "모든 원숭이들의 영웅이 되었다.", 7, 1)
        me:dialog(npc, "일전엔 고마웠네.", false, false)
        return
    end

    if main_step == 36 then
        me:dialog(npc, "일전엔 고마웠네.", false, false)
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
