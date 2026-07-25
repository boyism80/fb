-- npc: 보름달

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, " 밝은 달이 떠 있다. 어쩐지 움직일 것 같다.", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
