-- @note Trash: 1_공통/진원관.txt "토원왕". Skull necklace sub2 (receive letter 1->2), sub3 (좌황활력환->과일나라).
-- Phase 1: sub-quests only. Phase 2: main 2->3, 28->29.

---@brief   Sub2 step 1 + 원숭이편지: take letter, set sub2=2.
local function do_sub2_receive_letter(me, npc)
    if not me:has_items("원숭이편지", 1) then
        return false
    end
    local b = me:dialog(npc, "아니, 이건 소원왕의 편지가 아니신가! 그 트이한 표식은 바로 나에게 가져다주시라고 쓰여져 계시는거다.", false, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "음, 수고하셨다. 자, 그럼 어디 한 번 읽어보실테다. 잘 지내시고 계시겠지? 별 일 없으셔야 하실텐데.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "편지는 잘 받았으니 소원왕에게도 안부 전해주시기 바란다. 잘 가실거라 믿으신다.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    if not me:rmitem("원숭이편지", 1, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
        return true
    end
    local q2 = me:quest(QUEST_SKULL_NECKLACE_2)
    if q2 then
        q2:step(2)
    end
    return true
end

---@brief   Sub3 step 0: intro, set sub3=1.
local function do_sub3_start(me, npc)
    local sel, btn = me:list(npc, "고민이시다... 정말 고민이시다...", { "무슨 일이신가요?" }, false)
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 0 then
        return false
    end
    local b = me:dialog(npc, "요즘 원숭이들이 병에 걸려서 고민이시다. 정말로 고민이시다.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "이미 병에 걸리신 원숭이들을 치료할 방법은 없으시지만, 이상한 약을 드시면 예방은 하실 수 있다고들 하신다.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "그 약을 좀 구해다주셔야한다.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    local q3 = me:quest(QUEST_SKULL_NECKLACE_3)
    if q3 == nil then
        q3 = me:start_quest(QUEST_SKULL_NECKLACE_3)
        if q3 == nil then
            return true
        end
    end
    if q3 then
        q3:step(1)
    end
    return true
end

---@brief   Sub3 step 2: take 좌황활력환, give 과일나라, set sub3=3.
local function do_sub3_turnin(me, npc)
    if not me:has_items("좌황활력환", 1) then
        me:dialog(npc, "퀘스트 오류입니다.\n사진을 찍어 홈페이지에 문의하세요.", false, false)
        return true
    end
    local b = me:dialog(npc, "정말 수고하셨다! 어서 이걸 원숭이들에게 복용시켜야 하시겠다. 다만 양이 좀 더 많으셨으면 더 좋으셨다.", false, true)
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "최대한 절약하셔서 많이 나누어 드셔야 하시겠다. 고마우시다, 이건 내가 몰래 숨겨 가지고 계시던 음식이시다.", true, true)
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["좌황활력환"] = 1 } },
        { ['item'] = { ["과일나라"] = 1 } }
    )
    if code == EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", false, false)
        return true
    elseif code == EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 과일나라를 줄 수 없습니다.", false, false)
        return true
    end
    local q3 = me:quest(QUEST_SKULL_NECKLACE_3)
    if q3 then
        q3:step(3)
    end
    return true
end

function NPC_465(me, npc)
    local q2 = me:quest(QUEST_SKULL_NECKLACE_2)
    local q3 = me:quest(QUEST_SKULL_NECKLACE_3)

    -- Sub2: step 1 + letter -> step 2
    if q2 and q2:step() == 1 then
        if do_sub2_receive_letter(me, npc) then
            return
        end
    end

    -- Sub3: step 0 -> start
    if q3 == nil or q3:step() == 0 then
        if do_sub3_start(me, npc) then
            return
        end
        return
    end

    if q3:step() == 1 then
        me:dialog(npc, "병을 예방하기 위해 이상한 약을 먹어야 하는데, 그 약을 구해다 주셔야 한다.", false, false)
        return
    end

    -- Sub3: step 2 -> turn in
    if q3:step() == 2 then
        if do_sub3_turnin(me, npc) then
            return
        end
        return
    end

    -- Phase 2: main 2->3 (의견), 28->29 (마른갈대)
    local main_q = me:quest(QUEST_SKULL_NECKLACE)
    if main_q then
        local s = main_q:step()
        if s == 2 then
            local b = me:dialog(npc, "음? 참원왕께서 의견이나 계책을 바사리고 계신다고? 그래, 마침 생각하시던 일이 계시는데 말이다.", false, true)
            if b == DIALOG_RESULT.QUIT then
                return
            end
            b = me:dialog(npc, "좌황활력환을 대량으로 좀 얻으실 수 있으시면 굉장한 도움이 되실것 같으시다. 이게 내 의견이시다.", true, true)
            if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
                return
            end
            main_q:step(3)
            return
        end
        if s == 3 then
            me:dialog(npc, "내 의견을 참원왕에게 전해주면 되신다.", false, false)
            return
        end
        if s == 28 then
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
            main_q:step(29)
            return
        end
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
