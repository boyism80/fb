-- npc: 비밀세작도우미
function NPC_51(me, npc)
    ::NPC_51_0001::
    local button = me:dialog(npc, '여기는 비밀세작의 집입니다. 85 레벨부터 99 레벨 이상 고레벨이 사냥하기에 좋습니다.', { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '일본밀정들이 등장하며, 얻을 수 있는 아이템은 호박, 각종 색호박들입니다.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_51_0001
    end
end
