-- npc: 단군청룡스승

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
