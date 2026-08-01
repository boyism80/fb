-- npc: 성문2

return {
    on_click = function(me, npc)
        local button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
