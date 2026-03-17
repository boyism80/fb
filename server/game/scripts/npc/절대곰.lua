
function NPC_459(me, npc)
    local button = me:dialog(npc, "zZz...쿨쿨...", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
