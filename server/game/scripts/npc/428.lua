-- npc: 회상틈새의현무

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
