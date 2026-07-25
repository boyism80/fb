-- npc: 초도

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "......", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
