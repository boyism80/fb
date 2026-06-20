local quest = require('lib.quest')
function NPC_369(me, npc)
    local q = me:quest(quest.QUEST_DOJAECHUN)
    local at_step_one = (q and q:step() == 1)

    if not at_step_one then
        local msg = string.format("오 %s 자넨가? 언제 한번 우리와 바둑이라도 한번 두면 좋겠구먼..", me:name())
        local button = me:dialog(npc, msg, false, false)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    if not me:has_items("신선의책", 1) then
        me:dialog(npc, "....", false, false)
        return
    end

    ::NPC_369_0000::
    local button = me:dialog(npc, "아니! 자네 그것은 신선의책이 아닌가!\n\n요즘 도삭산이 시끄럽더니, 자네가 신선의책을 가져가서였군?", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_369_0001::
    button = me:dialog(npc, "아.. 자네가 흩어진 신선의책을 다시 만들어왔다고?\n\n그래.. 내 자네를 도와주도록 하지.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_369_0000
    end

    button = me:dialog(npc, "지금 바로 도삭산 비밀의방으로 보내주겠네.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    local map = name2map("도삭산비밀의방")
    if map then
        me:map(map, { 6, 11 })
    end
end
