-- npc: 절대곰

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "zZz...쿨쿨...", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
