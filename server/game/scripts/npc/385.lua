-- npc: 설날교환원
function NPC_385(me, npc)
    local button = me:dialog(npc, "준비중입니다.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
