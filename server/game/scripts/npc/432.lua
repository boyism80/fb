-- npc: 현무의육체

return {
    ON_CLICK = function(me, npc)
        if me:class() ~= CLASS.WARRIOR then
            me:dialog(npc, ".....zZZ", { prev = false, next = false })
            return
        end
        me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", { prev = false, next = false })
    end
}
