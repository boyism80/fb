-- npc: 물망동도우미
function NPC_48(me, npc)
    ::NPC_48_0001::
    local button = me:dialog(npc, '여기는 물망동입니다. 80 레벨부터 99 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_48_0010::
    button = me:dialog(npc, '각종 도깨비와 도깨비불이 등장하며, 얻을 수 있는 아이템은 주로 호박입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_48_0001
    end
end
