-- @note Trash: 1_공통/진원관.txt "중원왕". Skull necklace sub4 (아기원숭이 give, then 2->3 진원창).
-- Phase 1: sub-quest only. Phase 2: main 34->35.

---@brief   Sub4 step 0: list intro, "도와드릴께요" -> give 아기원숭이, set sub4=1.
local function do_sub4_start(me, npc)
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
    local b = me:dialog(npc, "이 아이는 원래 우리 부족 아이가 아니시다. 이주하는 난리통에 어머니를 잃어버리신 아이다. 책임지시고", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "이 아이의 부족을 찾아 데려다 주셔야 한다. 우리 원숭이들은 모두 4성에 흩어져 계시다! 힘들어도 꼭 찾아주셔야 한다!", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    me:mkitem("아기원숭이", 1)
    local q = me:quest(QUEST_SKULL_NECKLACE_4)
    if q == nil then
        q = me:start_quest(QUEST_SKULL_NECKLACE_4)
        if q == nil then
            return true
        end
    end
    if q then
        q:step(1)
    end
    return true
end

---@brief   Sub4 step 2: give 진원창, set sub4=3.
local function do_sub4_complete(me, npc)
    local b = me:dialog(npc, "수고하셨다. 사람들 중에서도 착한 마음씨를 가진 사람이 있으시긴 하셨다. 이걸 받아라. 섬에서 쓰시던 무기시다.", false, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "부하들과 같이 물고기 사냥 가실때 쓰셨던 물건이시다. 요긴하게 쓰셨으면 좋으시겠다.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    me:mkitem("진원창", 1)
    local q = me:quest(QUEST_SKULL_NECKLACE_4)
    if q then
        q:step(3)
    end
    return true
end

function NPC_466(me, npc)
    local q4 = me:quest(QUEST_SKULL_NECKLACE_4)

    if q4 == nil or q4:step() == 0 then
        if do_sub4_start(me, npc) then
            return
        end
        return
    end

    if q4:step() == 1 then
        me:dialog(npc, "이 아이의 부족을 찾아 데려다 주셔야 한다.", false, false)
        return
    end

    if q4:step() == 2 then
        if do_sub4_complete(me, npc) then
            return
        end
        return
    end

    -- Phase 2: main 34->35 (마른갈대)
    local main_q = me:quest(QUEST_SKULL_NECKLACE)
    if main_q and main_q:step() == 34 then
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
        main_q:step(35)
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
