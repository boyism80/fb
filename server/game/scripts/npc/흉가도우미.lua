function NPC_49(me, npc)
    ::NPC_49_COS00::
    local button = me:dialog(npc, '여기는 흉가입니다. 95 레벨부터 99 레벨 이상 고레벨이 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '각종 귀신 몬스터들이 등장하며, 얻을 수 있는 아이템은 진호박이고, 성에 따라 각종 색호박들도 얻을 수 있습니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_49_COS00
    end
end