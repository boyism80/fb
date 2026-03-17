function NPC_46(me, npc)
    ::NPC_46_0001::
    local button = me:dialog(npc, '여기는 전갈굴입니다. 55 레벨부터 70 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '전갈과 가재 등의 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 호박입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_46_0001
    end
end