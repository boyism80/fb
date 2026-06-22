-- npc: 주작의육체
function NPC_430(me, npc)
    if me:class() ~= CLASS.MAGE then
        me:dialog(npc, ".....zZZ", false, false)
        return
    end
    me:dialog(npc, "......신의 육체는 특별한 향료로 깨울 수 있으니...", false, false)
end
