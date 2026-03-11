
local function do_sub4_return_baby(me, npc)
    if not me:has_items("아기원숭이", 1) then
        me:dialog(npc, "퀘스트 오류입니다.\n아기원숭이 아이템이 없습니다.", false, false)
        return true
    end
    ::NPC_467_0040::
    local b = me:dialog(npc, "잠깐만! 자네가 안고 계시는 그 아기 원숭이는 누구의 자식이시지? 뭐? 어머니를 찾고 계시다고?", false, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_467_0041::
    b = me:dialog(npc, "밥도 안드시고 실의에 빠져계시는 원숭이 부부가 있으셨다. 이런 기쁘실데가 있나! 수고 하셨다!", true, true)
    if b == DIALOG_RESULT.PREV then
        goto NPC_467_0040
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    if not me:rmitem("아기원숭이", 1, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
        return true
    end
    local q4 = me:quest(QUEST_SKULL_NECKLACE_4)
    if q4 then
        q4:step(2)
    end
    return true
end

local function do_sub5_start(me, npc)
    ::NPC_467_0010::
    local sel, btn = me:list(npc, "요즘따라 인간들을 자주 만나시게 되시는도다.", { "이름을 가르쳐주세요.", "이곳의 상황은 좀 어떤가요?", "제가 도와드릴 일은 없을까요?" }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil then
        return false
    end
    if sel == 0 then
        me:dialog(npc, "나의 이름은 알아서 뭐하시려고 그러시는가?", false, false)
        return true
    end
    if sel == 1 then
        me:dialog(npc, "이 곳의 상황은 본래와 다를바 없으시다.", false, false)
        return true
    end
    ::NPC_467_0020::
    sel, btn = me:list(npc, "음? 하하하. 고마운 말씀이시다. 진심이신가?", { "네, 꼭 도와드릴께요.", "아뇨, 그만 둘래요." }, true)
    if btn == DIALOG_RESULT.PREV then
        goto NPC_467_0010
    end
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        me:dialog(npc, "장난을 치는 사람은 싫어한다.", false, false)
        return true
    end
    ::NPC_467_0030::
    local b = me:dialog(npc, "요즘 날쌔시고 유능하신 부하들을 시켜서서 다른 왕들과 편지를 주고받으시고 계신데, 그만 이번에 중요한 편지", true, true)
    if b == DIALOG_RESULT.PREV then
        goto NPC_467_0020
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_467_0031::
    b = me:dialog(npc, "하나가 흉폭해진 원숭이들 손에 떨어지고 마셨다. 부하들의 말에 따르면 요 앞에서 잃어버리셨다고하니, 우리", true, true)
    if b == DIALOG_RESULT.PREV then
        goto NPC_467_0030
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_467_0032::
    b = me:dialog(npc, "부족이 사시는 이 건물안에 남아계신 것은 확실하시다. 자네가 어떻게 좀 찾아주셨으면 좋으시겠다.", false, true)
    if b == DIALOG_RESULT.PREV then
        goto NPC_467_0031
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    local q5 = me:quest(QUEST_SKULL_NECKLACE_5)
    if q5 == nil then
        q5 = me:start_quest(QUEST_SKULL_NECKLACE_5)
        if q5 == nil then
            return true
        end
    end
    if q5 then
        q5:step(1)
    end
    return true
end

local function do_sub5_turnin(me, npc)
    if not me:has_items("원숭이편지", 1) then
        me:dialog(npc, "부족이 사시는 이 건물안에 남아계신 것은 확실하시다.", false, false)
        return true
    end
    ::NPC_467_0050::
    local b = me:dialog(npc, "이런 편지를 찾아오시다니 엄청난 실력이시군. 도대체 어디서 찾으신건가 믿을 수가 없으시다.", false, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_467_0051::
    b = me:dialog(npc, "분명 이걸 찾아오시느라 많은 고생을 하셨겠지? 자, 이걸 드시면 좋으시다. 몸이 허하실때 드시는", false, true)
    if b == DIALOG_RESULT.PREV then
        goto NPC_467_0050
    end
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["원숭이편지"] = 1 } },
        { ['item'] = { ["바나나다발"] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
        return true
    end
    if code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 바나나다발을 받을 수 없습니다.", false, false)
        return true
    end
    local q5 = me:quest(QUEST_SKULL_NECKLACE_5)
    if q5 then
        q5:step(2)
    end
    return true
end

function NPC_467(me, npc)
    local q4 = me:quest(QUEST_SKULL_NECKLACE_4)
    local q5 = me:quest(QUEST_SKULL_NECKLACE_5)

    if q4 and q4:step() == 1 then
        if do_sub4_return_baby(me, npc) then
            return
        end
    end

    if q5 == nil or q5:step() == 0 then
        if do_sub5_start(me, npc) then
            return
        end
        return
    end

    if q5:step() == 1 then
        if do_sub5_turnin(me, npc) then
            return
        end
        return
    end

    local main_q = me:quest(QUEST_SKULL_NECKLACE)
    if main_q and main_q:step() == 29 then
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
        main_q:step(30)
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
