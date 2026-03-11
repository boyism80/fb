
local function do_sub9_start(me, npc)
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
    sel, btn = me:list(npc, "음? 하하하. 고마운 말씀이시다. 진심이신가?", { "네, 꼭 도와드릴께요.", "아뇨, 그만 둘래요." }, true)
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        me:dialog(npc, "장난을 치는 사람은 싫어한다.", false, false)
        return true
    end
    ::NPC_471_0001::
    local b = me:dialog(npc, "내가 이번에 새로 술을 담그셨다. 친구에게 술을 주고 싶으시다.", true, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_471_0002::
    b = me:dialog(npc, "하지만, 이런 비상시에 왕이 자리를 비우실 수는 없으셔서 곤란하셨던 참이시다.", true, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_471_0001
    end
    ::NPC_471_0003::
    b = me:dialog(npc, "자, 이 술을 내 친구 화원왕에게 가져다 주시면 끝이시다. 잘 부탁하신다!", true, false)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_471_0002
    end
    me:mkitem("특제원숭이술", 1)
    local q9 = me:quest(QUEST_SKULL_NECKLACE_9)
    if q9 == nil then
        q9 = me:start_quest(QUEST_SKULL_NECKLACE_9)
        if q9 == nil then
            return true
        end
    end
    if q9 then
        q9:step(1)
    end
    return true
end

local function do_sub9_complete(me, npc)
    ::NPC_471_0004::
    local b = me:dialog(npc, "아, 정말 수고하셨다. 보답으로 이걸 드리시겠다. 내가 털을 다듬으실때, 쓰시는 물건이시다.", false, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    ::NPC_471_0005::
    b = me:dialog(npc, "나는 워낙 미남이시라 더이상 필요없으시니까. 자네가 요긴하게 쓰시면 좋으시겠다.", true, false)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    if b == DIALOG_RESULT.PREV then
        goto NPC_471_0004
    end
    me:mkitem("거울방패", 1)
    local q9 = me:quest(QUEST_SKULL_NECKLACE_9)
    if q9 then
        q9:step(3)
    end
    return true
end

function NPC_471(me, npc)
    local q9 = me:quest(QUEST_SKULL_NECKLACE_9)

    if q9 == nil or q9:step() == 0 then
        if do_sub9_start(me, npc) then
            return
        end
        return
    end

    if q9:step() == 1 then
        me:dialog(npc, "이 술을 내 친구 화원왕에게 가져다 주시면 끝이시다. 잘 부탁하신다!", false, false)
        return
    end

    if q9:step() == 2 then
        if do_sub9_complete(me, npc) then
            return
        end
        return
    end

    local main_q = me:quest(QUEST_SKULL_NECKLACE)
    if main_q and main_q:step() == 32 then
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
        main_q:step(33)
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
