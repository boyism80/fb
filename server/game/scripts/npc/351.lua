-- npc: 촌장

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "안녕하세요.. 여기는 세시마을 입니다. 저는 촌장 백도원이라고 합니다.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
