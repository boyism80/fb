-- npc: 화원왕
local quest = require('lib.quest')
local enum = require('lib.enum')
local function do_sub9_receive_wine(me, npc)
    if not me:has_items("특제원숭이술", 1) then
        me:dialog(npc, "퀘스트 오류입니다.\n특제원숭이술 아이템이 없습니다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "음? 목원왕의 선물이시라고? 이런이런 생각도 못하셨었다. 이런 좋지 않으실때 위로를 해주시는건 역시 친구밖에 안계신다.", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "고마우시다. 잘 받으셨다고 내 인사를 전해주셔야 좋으시다. 이정도면 우리 부족 원숭이들이 한모금씩 맛을 보실 수 있으시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    if not me:rmitem("특제원숭이술", 1, ITEM_DELETE_TYPE.GIVE) then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
        return true
    end
    local q9 = me:quest(quest.QUEST_SKULL_NECKLACE_9)
    if q9 then
        q9:step(2)
    end
    return true
end

local function do_sub7_start(me, npc)
    local sel, btn = me:list(npc, "요즘따라 인간들을 자주 만나시게 되시는도다.", { "이름을 가르쳐주세요.", "이곳의 상황은 좀 어떤가요?", "제가 도와드릴 일은 없을까요?" }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil then
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
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
        me:dialog(npc, "장난을 치는 사람은 싫어한다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "우리 부족의 아이가 한 명 쓰러지셨다. 다행히 흉폭해지시는 병은 아니시고, 단순한 열병이시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "나로서는 이 병을 치료해주실 능력이 없으시다. 우리 원숭이의 여러 부족들 중에 단 한 부족의", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "우두머리만이 열병을 치료하시는 능력을 가지고 계시다. 그 우두머리를 찾으셔서 아이를 치료하시는게 좋으시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    me:mkitem("아픈아기원숭이", 1)
    local q7 = me:quest(quest.QUEST_SKULL_NECKLACE_7)
    if q7 == nil then
        q7 = me:start_quest(quest.QUEST_SKULL_NECKLACE_7)
        if q7 == nil then
            return true
        end
    end
    if q7 then
        q7:step(1)
    end
    return true
end

local function do_sub7_complete(me, npc)
    if not me:has_items("건강한아기원숭이", 1) then
        me:dialog(npc, "퀘스트 오류입니다\n건강한아기원숭이 아이템이 없습니다.", { prev = false, next = false })
        return true
    end
    local b = me:dialog(npc, "하하하. 과연 건강하게 돌아오실 줄 알고 계셨다. 정말 수고하셨다! 그래, 이 아이의 웃음을 보고 계시면", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "참, 이건 작은 답례이시다. 이 근처에서 얻으신 물건이시다. 어디다 쓰시는지 도통 아시다가도 모르겠지만 말이시다.", { prev = true, next = true })
    if b == DIALOG_RESULT.PREV or b == DIALOG_RESULT.QUIT then
        return false
    end
    local code = me:exchange(
        { ['item'] = { ["건강한아기원숭이"] = 1 } },
        { ['item'] = { ["초코바나나"] = 1 } }
    )
    if code == enum.EXCHANGE_RESULT.LACK_COST then
        me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
        return true
    elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
        me:dialog(npc, "소지품이 가득 차서 초코바나나를 줄 수 없습니다.", { prev = false, next = false })
        return true
    end
    local q7 = me:quest(quest.QUEST_SKULL_NECKLACE_7)
    if q7 then
        q7:step(3)
    end
    return true
end

return {
    on_click = function(me, npc)
        local q9 = me:quest(quest.QUEST_SKULL_NECKLACE_9)
        local q7 = me:quest(quest.QUEST_SKULL_NECKLACE_7)

        if q9 and q9:step() == 1 then
            if do_sub9_receive_wine(me, npc) then
                return
            end
        end

        if q7 == nil or q7:step() == 0 then
            if do_sub7_start(me, npc) then
                return
            end
            return
        end

        if q7:step() == 1 then
            me:dialog(npc, "한 우두머리만이 원숭이를 치료할 수 있는 능력을 가지고 있으시다.", { prev = false, next = false })
            return
        end

        if q7:step() == 2 then
            if do_sub7_complete(me, npc) then
                return
            end
            return
        end

        local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
        if main_q then
            local s = main_q:step()
            if s == 23 then
                ::NPC_469_0001::
                local b = me:dialog(npc, "참원왕이 의견을 물으셨다고?", { prev = false, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                ::NPC_469_0002::
                b = me:dialog(npc, "더 큰 문제가 계시다. 바로 우리 어린 원숭이들에 대한 문제이시다. 이 곳으로 옮겨오신지 얼마 되지 않으신 탓도", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_469_0001
                end
                ::NPC_469_0003::
                b = me:dialog(npc, "계시지만, 아직 이 땅이 낯설으셔서 편하게 주무실 수가 없으시다. 얼마전에 보셨는데, 담벼락 밖에서 병사 한 명이", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_469_0002
                end
                ::NPC_469_0004::
                b = me:dialog(npc, "'마른갈대'라는 것을 깔고 그 위에 누워 주무시던데.. 아주 편해보이셨다. 어디서 나시는 건지는 모르시지만, 그게 계시면", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_469_0003
                end
                ::NPC_469_0005::
                b = me:dialog(npc, "적어도 어린 원숭이들이 편하게 주무실 수 있으실거 같으시다. 이런 내 생각을 참원왕께 전해주시면 좋으시다.", { prev = true, next = true })
                if b == DIALOG_RESULT.QUIT then
                    return
                end
                if b == DIALOG_RESULT.PREV then
                    goto NPC_469_0004
                end
                main_q:step(24)
                return
            end
            if s == 24 then
                me:dialog(npc, "이런 의견을 빨리 참원왕에게 전해주시면 좋으시다.", { prev = false, next = false })
                return
            end
            if s == 26 then
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
                main_q:step(27)
                return
            end
        end

        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    end
}
