
function NPC_454(me, npc)
    local button = me:dialog(npc, "안녕하신가, " .. me:name() .. ". 나는 이곳 제단에서 힘을 모아 다른 제단지기들에게 [용의 축복]을 걸어주는 청룡제단지기라네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, " 내가 천도복숭아를 먹고, 축복을 외울만한 힘이 [750 마력]만큼 모이게 되면,\n [용의 축복]을 다른 제단지기들에게 걸어주고, 제단지기들의 마력을 남은양의 50% 만큼 채워 준다네.", false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
