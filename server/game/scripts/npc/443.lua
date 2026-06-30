-- npc: 고구려갑판선원
function NPC_443(me, npc)
    local button = me:dialog(npc, "준비중입니다.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
