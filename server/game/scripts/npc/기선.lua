local server = require('lib.server')

function NPC_289(me, npc)
    local button = me:dialog(npc, "옛날 함께 바둑을 두던 그 놈이 그립군... 홀홀...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if not me:has_items("황매주", 1) then
        return
    end
    button = me:dialog(npc, string.format("오! %s, 한수가 보내서 왔구만... 그 친구 요즘 근황이 어떠하던가?", me:name()), false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local code = me:exchange(
        { ['item'] = { ["황매주"] = 1 } },
        { ['item'] = { ["용의골무"] = 1 } }
    )
    if code == server.EXCHANGE_RESULT.LACK_COST then
        return
    end
    if code == server.EXCHANGE_RESULT.LACK_CAPACITY then
        return
    end

    button = me:dialog(npc, "황매주를 보니, 그 친구 생각이 나는구만...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "이 술맛은 정말 일품이란 말이야...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "허허, 황매주는 고맙게 받겠네. 그 대신 내가 가진게 있는데, 어디보자...", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    button = me:dialog(npc, "여기 [용의골무]를 가져가게나. 조심히 들어가게~", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end
