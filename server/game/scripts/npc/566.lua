-- npc: 마리
function NPC_566(me, npc)
    local button = me:dialog(npc, "준비중입니다.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
