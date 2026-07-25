-- npc: 막걸리상인

return {
    on_click = function(me, npc)
        local item_name = "막걸리"
        if me:has_items(item_name, 1) then
            local button = me:dialog(npc, string.format("%s님은 이미 %s를 가지고 계시네요.", me:name(), item_name), { prev = false, next = false })
            if button == DIALOG_RESULT.QUIT then
                return
            end
            return
        end

        local sel, btn = me:list(npc, string.format("[%s]가 필요하신 분들에게 무료로 하나씩 나눠드리고 있어요. ", item_name), {"네. 하나만 주세요.", "아니요. 필요 없어요."})
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if sel == nil then
            return
        end

        if sel == 1 then
            if me:mkitem(item_name, 1) == nil then
                return
            end
            local button = me:dialog(npc, "자, 여기 하나 드릴게요.", { prev = false, next = false })
            if button == DIALOG_RESULT.QUIT then
                return
            end
        end
    end
}
