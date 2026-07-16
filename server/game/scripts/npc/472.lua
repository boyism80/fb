-- npc: 진봉쇄장군
local quest = require('lib.quest')
local function do_sub3_give_medicine(me, npc)
    local sel, btn = me:list(npc, "모험가가 나에겐 무슨 일인가?", { "원숭이들이 약이 필요하다고 합니다." }, { prev = false })
    if btn == DIALOG_RESULT.QUIT or sel == nil or sel ~= 1 then
        return true
    end
    ::NPC_472_0010::
    local b = me:dialog(npc, "음, 아마도 우리 군인들이 쓰는 좌황활력환을 말하는 것 같은데? 원숭이가 별걸 다 알고 있군.", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    b = me:dialog(npc, "얼마나 효능이 있을런지는 모르지만, 모른 척 하는 것도 못할 짓이지. 자 받으시게나.", { prev = false, next = true })
    if b == DIALOG_RESULT.QUIT then
        return false
    end
    me:mkitem("좌황활력환", 1)
    local q = me:quest(quest.QUEST_SKULL_NECKLACE_3)
    if q then
        q:step(2)
    end
    return true
end

function NPC_472(me, npc)
    local q3 = me:quest(quest.QUEST_SKULL_NECKLACE_3)

    if q3 and q3:step() == 1 then
        if do_sub3_give_medicine(me, npc) then
            return
        end
        return
    end

    local main_q = me:quest(quest.QUEST_SKULL_NECKLACE)
    if main_q then
        local s = main_q:step()
        if s == 4 then
            ::NPC_472_0020::
            local b = me:dialog(npc, "좌황활력환이 또 필요하다는 것인가?\n그리 쉽게 내줄 수 있는 물건이 아니네.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            b = me:dialog(npc, "미안하지만, 사정해도 도와줄 수 없네.\n그만 돌아가게나.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            main_q:step(5)
            return
        end
        if s == 5 then
            me:dialog(npc, "미안하지만, 사정해도 도와줄 수 없네.", { prev = false, next = false })
            return
        end
        if s == 6 then
            if not me:has_items("원숭이의편지", 1) then
                me:dialog(npc, "퀘스트 오류입니다.\n원숭이의편지 아이템이 없습니다.", { prev = false, next = false })
                return
            end
            ::NPC_472_0030::
            local b = me:dialog(npc, "(편지를 읽는 중이다.)", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_472_0031::
            b = me:dialog(npc, "이런 참혹한 일이 벌어지고 있단 말인가! 아아, 인간의 양심을 가지고서 이런 참혹한 광경을 외면 할 수는 없는 일이야.", { prev = true, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            if b == DIALOG_RESULT.PREV then
                goto NPC_472_0030
            end
            b = me:dialog(npc, "알겠네, 내가 왕께 말씀드려 좌황활력환을 있는 힘껏 원숭이들에게 건네주도록 하겠네.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            if not me:rmitem("원숭이의편지", 1, ITEM_DELETE_TYPE.GIVE) then
                me:dialog(npc, "아이템을 제거할 수 없습니다.", { prev = false, next = false })
                return
            end
            main_q:step(7)
            return
        end
        if s == 7 then
            me:dialog(npc, "알겠네, 내가 왕께 말씀드려 좌황활력환을 있는 힘껏 원숭이들에게 건네주도록 하겠네.", { prev = false, next = false })
            return
        end
    end

    me:dialog(npc, "준비중입니다.", { prev = false, next = false })
end