-- npc: 백호갑옷상
function NPC_321(me, npc)
    local button = me:dialog(npc, "준비중입니다.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
