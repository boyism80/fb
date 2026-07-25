-- npc: 날씨알려주는돌

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "돌이 젖었으면 비\n돌 위에 하야면 눈\n돌이 안보이면 안개\n돌이 흔들리면 지진\n돌이 없으면 태풍", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local sel, btn = me:list(npc, "...", {"오늘의 날씨는???"})
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
            if button == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
}
