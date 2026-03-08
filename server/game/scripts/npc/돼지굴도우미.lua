function NPC_37(me, npc)
    ::NPC_37_COS00::
    local button = me:dialog(npc, '여기는 돼지굴입니다. 30 레벨부터 45 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '각종 돼지들이 등장하며, 얻을 수 있는 아이템은 주로 산돼지고기입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_37_COS00
    end
end