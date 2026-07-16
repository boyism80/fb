-- npc: 중독된폐한
function NPC_611(me, npc)
    local button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
