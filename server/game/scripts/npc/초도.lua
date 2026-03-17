
function NPC_604(me, npc)
    local button = me:dialog(npc, "......", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
