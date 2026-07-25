-- npc: 자호굴도우미

return {
    ON_CLICK = function(me, npc)
        ::NPC_50_0001::
        local button = me:dialog(npc, '여기는 자호굴입니다. 40 레벨부터 60 레벨까지 사냥하기에 좋습니다.', { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_50_0002::
        button = me:dialog(npc, '자호와 친자호 등의 몬스터들이 등장하며, 얻을 수 있는 아이템은 주로 자호의가죽입니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_50_0001
        end
    end
}
