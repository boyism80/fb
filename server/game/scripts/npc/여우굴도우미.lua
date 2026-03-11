function NPC_39(me, npc)
    ::NPC_39_0001::
    local button = me:dialog(npc, '여기는 여우굴입니다. 30 레벨부터 45 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_39_0002::
    button = me:dialog(npc, '여우, 구미호 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 여우모피입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_39_0001
    end
end