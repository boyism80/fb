-- npc: 부롱대미궁입장

return {
    on_click = function(me, npc)
        local sel = me:list(npc, "부롱대미궁에 입장하실래요?", {"예", "아니오"})
        if sel == nil then
            return
        end

        if sel == 1 then
            local map = name2map("부롱대미궁")
            if map then
                me:map(map, math.random(1, 6), math.random(1, 6))
            end
            return
        end

        if sel == 2 then
            local button = me:dialog(npc, "잘 생각하셨어요. 준비가 되거든 다시 오세요.", { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
}
