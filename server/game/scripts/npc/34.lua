-- npc: 초보사냥터도우미
function NPC_34(me, npc)
    ::NPC_34_0001::
    local button = me:dialog(npc, '여기는 초보자사냥터입니다. 초보자에게 적합한 던전으로, 1 레벨부터 10 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_34_0002::
    button = me:dialog(npc, '주로 토끼, 다람쥐 등의 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 도토리, 토끼고기입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_34_0001
    end

    ::NPC_34_0003::
    button = me:dialog(npc, '토끼와 다람쥐 이외의 다른 몬스터들은 보기보다 강하니까 주의하세요.', false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_34_0002
    end
end
