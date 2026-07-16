-- npc: 백호의육체
function NPC_429(me, npc)
    if me:class() ~= CLASS.ROGUE then
        me:dialog(npc, ".....zZZ", { prev = false, next = false })
        return
    end
    me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", { prev = false, next = false })
end
