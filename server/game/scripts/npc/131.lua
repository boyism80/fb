-- npc: 환상의섬주모
function NPC_131(me, npc)
    local button = me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
