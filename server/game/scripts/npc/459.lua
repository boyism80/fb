-- npc: 절대곰
function NPC_459(me, npc)
    local button = me:dialog(npc, "zZz...쿨쿨...", { prev = false, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
