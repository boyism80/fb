-- npc: 대원왕
local quest = require('lib.quest')
local enum = require('lib.enum')
local function do_sub7_cure(me, npc)
    if not me:has_items("아픈아기원숭이", 1) then
        me:dialog(npc, "퀘스트 오류입니다.\n아픈아기원숭이 아이템이 없습니다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "연락은 진작부터 듣고 계셨다. 자, 어서 아이를 이리 주셔야 좋으시다. 나의 힘으로 열병따윈 순식간에 치료해주신다!", { prev = false, next = true })
    if b ~= 1 then
        return false
    end
    b = me:dialog(npc, "나 대원왕의 힘을 보시는게 좋으시다! 하아아앙아아앗!", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    b = me:dialog(npc, "헉헉, 자, 씻으신듯이 열이 가라앉으셨다. 어서 이 기쁜 소식을 부모에게 전해주셔야 좋으시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["아픈아기원숭이"] = 1 } },
        { ['item'] = { ["건강한아기원숭이"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
        return true
    elseif code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 건강한아기원숭이를 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    local q7 = me:quest(quest.QUEST_SKULL_NECKLACE_7)
    if q7 then
        q7:step(2)
    end
    return true
end

local function do_sub1_start(me, npc)
    local b = me:dialog(npc, "경계하실 것 없다. 나는 아직 미치지 않으셨으니까. 그래도 우리 아이들이 저렇게 날뛰는 이유는", { prev = false, next = true })
    if b ~= 1 then
        return false
    end
    b = me:dialog(npc, "찌꺼기 과일을 가져오신 인간들에게 있으시다. 반성하셔야 한다. 이 땅에 오신 이상, 우리도 적응하셔야 하지만,", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    b = me:dialog(npc, "무작정 이 땅의 과일만 주시는건 해서는 안될 일이시다. 아이들을 잃으셔서 가슴이 아프시다. 아프시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    local sel, btn = me:list(npc, "요즘따라 인간들을 자주 만나시게 되시는도다.", { "이름을 가르쳐주세요.", "이곳의 상황은 좀 어떤가요?", "제가 도와드릴 일은 없을까요?" }, { prev = true })
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if sel == 1 then
        me:dialog(npc, "나의 이름은 알아서 뭐하시려고 그러시는가?", { prev = false, next = false })
        return true
    end
    if sel == 2 then
        me:dialog(npc, "이 곳의 상황은 본래와 다를바 없으시다.", { prev = false, next = false })
        return true
    end
    sel, btn = me:list(npc, "음? 하하하. 고마운 말씀이시다. 진심이신가?", { "네, 꼭 도와드릴께요.", "아뇨, 그만 둘래요." }, { prev = true })
    if btn == DIALOG_RESULT.QUIT or sel ~= 1 then
        me:dialog(npc, "장난을 치는 사람은 싫어한다.", { prev = false, next = false })
        return true
    end
    b = me:dialog(npc, "고구려나 부여에서는 얼음이라는걸 구하실 수가 있으시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    b = me:dialog(npc, "투명하시고 차가우시면서도 딱딱하신 물체라고 하시는데... 뜨거운 섬에서 자라나신 나로서는 상상조차", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    b = me:dialog(npc, "할 수가 없으시다. 나에게 그 얼음이라는걸 보여주시면 좋으시다. 깨끗하시고 투명하신 분으로 말이시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    local q1 = me:quest(quest.QUEST_SKULL_NECKLACE_1)
    if q1 == nil then
        q1 = me:start_quest(quest.QUEST_SKULL_NECKLACE_1)
        if q1 == nil then
            me:dialog(npc, "퀘스트를 시작할 수 없습니다.", { prev = false, next = false })
            return true
        end
    end
    if q1 then
        q1:step(1)
    end
    return true
end

local function do_sub1_turnin(me, npc)
    if not me:has_items("깨끗한얼음", 1) then
        me:dialog(npc, "나에게 그 얼음이라는걸 보여주시면 좋으시다. 깨끗하시고 투명하신 분으로 말이시다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "오오. 이런 신기하신 것이 다 계실 줄이야! 이 곳은 정말 놀라움으로 가득찬 곳이시다. 하하하하하.", { prev = false, next = true })
    if b ~= 1 then
        return false
    end
    b = me:dialog(npc, "과연과연이시다. 생김새도 아름다우시고, 입에 넣으시면 시원시원하시고, 손에 쥐고계시면 차가우시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    b = me:dialog(npc, "자, 이건 내가 드리시는 답례이시다. 이 땅에서는 보기 힘드신 과일로 만드신 음식이시다. 맛있게 드시면 좋으시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV then
        return false
    end
    if b == DIALOG_RESULT.QUIT or not b then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["깨끗한얼음"] = 1 } },
        { ['item'] = { ["오도독망고과편"] = 1 } }
    )
    if code == enum.exchange_result.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
        return true
    elseif code == enum.exchange_result.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 오도독망고과편을 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    local q1 = me:quest(quest.QUEST_SKULL_NECKLACE_1)
    if q1 then
        q1:step(2)
    end
    return true
end

return {
    on_click = function(me, npc)
        local q7 = me:quest(quest.QUEST_SKULL_NECKLACE_7)
        local q1 = me:quest(quest.QUEST_SKULL_NECKLACE_1)

        if q7 and q7:step() == 1 then
            if do_sub7_cure(me, npc) then
                return
            end
        end

        if q1 == nil or q1:step() == 0 then
            if do_sub1_start(me, npc) then
                return
            end
            return
        end

        if q1 and q1:step() == 1 then
            if do_sub1_turnin(me, npc) then
                return
            end
            return
        end

        local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
        if main_q then
            local s = main_q:step()
            if s == 8 then
                ::NPC_463_0001::
                local b = me:dialog(npc, "참원왕께서 보내셨다고? 내 의견은 대부분의 원숭이들이", { prev = false, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                ::NPC_463_0002::
                b = me:dialog(npc, "이 나라의 언어를 모르고 계시다는거다. 나는 내 능력으로 대강 필요한 재료를 느끼고 계시는데, 이 땅에는", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_463_0001
                end
                ::NPC_463_0003::
                b = me:dialog(npc, "그런 재료들이 계시는지, 계시다면 그게 무엇이신지 알아내시려면 글자를 아셔야만 하신다. 그게 가장 큰 문제이시다.", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_463_0002
                end
                main_q:step(9)
                return
            end
            if s == 9 then
                me:dialog(npc, "참원왕에게 내 의견을 전달해주시면 되시다.", { prev = false, next = false })
                return
            end
            if s == 12 then
                if not me:has_items("창힐독본", 1) then
                    me:dialog(npc, "퀘스트 오류입니다.\n창힐독본이 없습니다.", { prev = false, next = false })
                    return
                end
                ::NPC_463_0004::
                local b = me:dialog(npc, "으음, 과연과연이시다. 음...이 창힐독본에 따르시면, 내가 생각하시고 계신 재료들의 이름은...", { prev = false, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                ::NPC_463_0005::
                b = me:dialog(npc, "기린의 피 2병, 악어의 피 2병, 그리고 깨끗한 얼음 5개시다. 이렇게 모으셔서 조합하시면", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_463_0004
                end
                ::NPC_463_0006::
                b = me:dialog(npc, "진원비전탕이 완성되신다. 그걸 두 개 만드셔서 가져다 주시면 좋으시다.", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_463_0005
                end
                if not me:rmitem("창힐독본", 1, ITEM_DELETE_TYPE.GIVE) then
                    me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                    return
                end
                main_q:step(13)
                return
            end
            if s == 13 then
                if not me:has_items("진원비전탕", 2) then
                    me:dialog(npc, "기린의 피 2병, 악어의 피 2병, 깨끗한 얼음 5개를 조합한 진원비전탕을 두 개 만들어서 가져다 주시면 좋으시다.", { prev = false, next = false })
                    return
                end
                ::NPC_463_0007::
                local b = me:dialog(npc, "이럴수가 계신가. 정말 만들어 주시리라 생각도 못하셨건만... 자네는 다시 안계실 훌륭한 사람이시다.", { prev = false, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                ::NPC_463_0008::
                b = me:dialog(npc, "돌아가시는 날까지 이 은혜를 잊지 않으시겠다. 고마우시다. 자, 그럼 이 남으신 한 병은 참원왕께 가져다주시면 좋으시다.", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_463_0007
                end
                if not me:rmitem("진원비전탕", 1, ITEM_DELETE_TYPE.GIVE) then
                    me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                    return
                end
                main_q:step(14)
                return
            end
            if s == 14 then
                me:dialog(npc, "남으신 한 병은 참원왕께 가져다주시면 고마우시다.", { prev = false, next = false })
                return
            end
            if s == 27 then
                if not me:has_items("마른갈대", 1) then
                    me:dialog(npc, "마른갈대를 구해서 왕들에게 하나씩 나누어 주게.", { prev = false, next = false })
                    return
                end
                local b = me:dialog(npc, "마른 갈대를 나눠주고 있다고 들었네. 수고하는 모습이 참 보기 좋군. 더 수고해주게.", { prev = false, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if not me:rmitem("마른갈대", 1, ITEM_DELETE_TYPE.GIVE) then
                    me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                    return
                end
                main_q:step(28)
                return
            end
        end

        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    end
}
