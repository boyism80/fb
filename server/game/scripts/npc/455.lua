-- npc: 현무제단지기
function NPC_455(me, npc)
    local button = me:dialog(npc, "안녕하신가, " .. me:name() .. ". 나는 이곳 제단에서 힘을 모아 사악한악령에게 [강력한 참격]을 가하는 현무제단지기라네.", { prev = false, next = true })
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, " 내가 천도복숭아를 먹고, 일격을 가할만한 힘이 [1500 마력]만큼 모이게 되면,\n [강력한 참격]을 사악한 악령에게 가하고, 대상에게 10의 데미지를 가하게 된다네.", { prev = false, next = false })
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
