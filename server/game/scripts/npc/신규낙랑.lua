
function NPC_279(me, npc)
    local quest = me:quest(QUEST_BEGINNER_PATH)
    if quest and (quest:step() == 7 or quest:completed()) then
        me:dialog(npc, "준비가 되셨으면 노란비서를 사용해 보세요.\n그럼, 안녕히 가십시오.", false, false)
        return
    end

    ::NPC_279_0001::
    local button = me:dialog(npc, "안녕하세요? 초보자방의 마지막까지 오시느라 수고하셨습니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_279_0002::
    button = me:dialog(npc, "이제는 이 좁은 초보자 방을 떠나, 드 넓은 바람의 나라 세계로 안내해 드리겠습니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_279_0001
    end

    ::NPC_279_0003::
    button = me:dialog(npc, "그 곳에서는 수천/수만의 사람들과 함께 게임을 즐길 수 있습니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_279_0002
    end

    ::NPC_279_0004::
    button = me:dialog(npc, "마지막으로 제가 노란비서 20개를 드리겠습니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_279_0003
    end

    ::NPC_279_0005::
    button = me:dialog(npc, "[노란비서]라는 아이템을 사용하시면 언제든지 [주막]으로 이동하시게 됩니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_279_0004
    end

    ::NPC_279_0006::
    button = me:dialog(npc, "단, 이것들은 1회용 이므로, 반드시 필요할 때만 사용하시기 바랍니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_279_0005
    end

    if quest == nil then
        quest = me:start_quest(QUEST_BEGINNER_PATH)
        if quest == nil then
            me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
            return
        end
    end
    quest:step(7)
    me:exp(me:exp() + 300)

    if me:mkitem("노란비서", 20) == nil then
        me:dialog(npc, "소지품이 가득 차서 노란비서를 줄 수 없습니다.", false, false)
        return
    end

    me:dialog(npc, "<노란비서>을 얻다!!!", false, false)
end
