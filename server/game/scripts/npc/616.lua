-- npc: 백두산북장인
function NPC_616(me, npc)
    local button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
