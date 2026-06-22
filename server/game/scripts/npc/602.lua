-- npc: 장안경비병
function NPC_602(me, npc)
    local button = me:dialog(npc, "여기는 황제 폐하가 계시는 곳입니다.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
