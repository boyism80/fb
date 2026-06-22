-- npc: 황금남부주작
function NPC_483(me, npc)
    local button = me:dialog(npc, "내게 도전하고 싶거든, 먼저 병사들을 모두 물리치고 오너라.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
