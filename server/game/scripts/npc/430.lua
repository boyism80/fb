-- npc: 주작의육체

return {
    on_click = function(me, npc)
        if me:class() ~= CLASS.MAGE then
            me:dialog(npc, ".....zZZ", { prev = false, next = false })
            return
        end
        me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", { prev = false, next = false })
    end
}
