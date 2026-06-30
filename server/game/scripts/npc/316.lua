-- npc: 주작수련스승
function NPC_316(me, npc)
    local button = me:dialog(npc, "준비중입니다.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
