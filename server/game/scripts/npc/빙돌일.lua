
function NPC_297(me, npc)
    local button = me:dialog(npc, "준비중입니다. 해당 기능 사용을 원하시면 홈페이지에 문의해 주세요.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
