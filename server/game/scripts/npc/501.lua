-- npc: 남팀워프

return {
    on_click = function(me, npc)
        if property("ns_start") ~= 1 then
            me:dialog(npc, "저런, 남북무한대전은 종료되었는데요. 나가는걸 도와드릴게요.", { prev = false, next = false })
            local map = name2map("국내성")
            if map then
                me:map(map, math.random(192, 202), math.random(103, 112))
            end
            return
        end

        local sel, btn = me:list(npc, string.format("%s님. 치열한 전장으로 바로 가고 싶으시다구요?", me:name()), {"네. 보내주세요.", "아니오. 겁나서..."}, { prev = false })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            me:map(129, math.random(13, 17), math.random(45, 47))
        end
    end
}
