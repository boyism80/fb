function NPC_35(me, npc)
    ::NPC_35_0001::
    local button = me:dialog(npc, '여기는 쥐굴입니다. 초보자에게 적합한 던전으로, 5 레벨부터 10 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_35_0002::
    button = me:dialog(npc, '주로 쥐, 박쥐 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 쥐고기입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_35_0001
    end

    ::NPC_35_0003::
    button = me:dialog(npc, '동굴의 끝에는 뱀굴이 숨겨져 있으니 잘 찾아보세요. 뱀굴은 10 레벨부터 20 레벨까지 사냥하기에 좋으며, 얻을 수 있는 아이템은 주로 뱀고기입니다.', false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_35_0002
    end
end