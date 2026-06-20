local quest = require('lib.quest')

function NPC_548(me, npc)
    local button = me:dialog(npc, "안녕하세요? 저는 백수인입니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if property("sesi_rightnow") ~= 2 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    local q = me:quest(quest.QUEST_BAEK_MONGYEON)
    if q == nil then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if q:step() < 1 then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    if me:has_items("연", 1) then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end

    button = me:dialog(npc, "중화절이 되니 몽연이형이 또 세시마을 대청소를 하는군요. 창고에서 또 뭐 꺼내달라고 할 텐데.. 귀찮은데..", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "연을 받아오라고 했다구요? 잠시만요.. 찾아볼께요....", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "............\n\n........여기있네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "혹시 또 뭐 더 찾아달라고 하면.. 직접와서 좀 찾으라고 해요.. 귀찮아요...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    me:mkitem("연", 1)
end
