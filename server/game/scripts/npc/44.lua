-- npc: 흑령굴도우미
function NPC_44(me, npc)
    ::NPC_44_0001::
    local button = me:dialog(npc, '여기는 흑령굴입니다. 65 레벨부터 90 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_44_0002::
    button = me:dialog(npc, '각종 유령 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 호박입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_44_0001
    end
end
