function NPC_40(me, npc)
    ::NPC_40_COS00::
    local button = me:dialog(npc, '여기는 인형굴입니다. 60 레벨부터 80 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '인형들과 인형술사가 등장하며, 얻을 수 있는 아이템은 호박과 진호박입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_40_COS00
    end
end