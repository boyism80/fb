function NPC_36(me, npc)
    ::NPC_36_COS00::
    local button = me:dialog(npc, '여기는 사슴굴입니다. 10 레벨부터 25 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '주로 사슴들이 등장하며, 얻을 수 있는 아이템은 주로 사슴고기입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_36_COS00
    end
end