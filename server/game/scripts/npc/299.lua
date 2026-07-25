-- npc: 빙돌삼

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "준비중입니다. 해당 기능 사용을 원하시면 홈페이지에 문의해 주세요.", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
