function ON_F1_EVENT(me)
    local npc = name2npc('낙랑')
    local button = me:dialog(npc, 'F1 이벤트 스크립트', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end

function ON_F2_EVENT(me)
    local npc = name2npc('낙랑')
    local button = me:dialog(npc, 'F2 이벤트 스크립트', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end