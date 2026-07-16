-- npc: 사마귀굴도우미
function NPC_45(me, npc)
    ::NPC_45_0001::
    local button = me:dialog(npc, '여기는 사마귀굴입니다. 55 레벨부터 70 레벨까지 사냥하기에 좋습니다.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_45_0010::
    button = me:dialog(npc, '거미와 사마귀 등의 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 호박입니다.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_45_0001
    end
end
