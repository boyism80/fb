-- npc: 주작제단지기

return {
    ON_CLICK = function(me, npc)
        local button = me:dialog(npc, "안녕하신가, " .. me:name() .. ". 나는 이곳 제단에서 힘을 모아 사악한악령에게 [파마의 중독]을 가하는 주작제단지기라네.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        button = me:dialog(npc, " 내가 천도복숭아를 먹고, 일격을 가할만한 힘이 [1000 마력]만큼 모이게 되면,\n [파마의 중독]을 사악한 악령에게 가한 뒤, 대상이 가지고 있는 현재 체력의 5% 만큼의 데미지를 입힐 수 있다네.", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
    end
}
