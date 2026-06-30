-- npc: 곰굴도우미
function NPC_38(me, npc)
    ::NPC_38_0001::
    local button = me:dialog(npc, '여기는 곰굴입니다. 20 레벨부터 40 레벨까지 사냥하기에 좋습니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    button = me:dialog(npc, '주로 곰, 호랑이 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 웅담, 호랑이고기입니다.', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_38_0001
    end
end
