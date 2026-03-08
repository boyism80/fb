-- @note Trash: 1_초보자의길\초보자도우미.txt "률". $초보자의길==7: warp only. Else: dialogs, exp 300, 노란비서 20, set 7, warp. 추천인등록 omitted.
--       Not the final script of 초보자의길: 아도 (0_초보지원) continues 7→8→9→10; step 10 = warp to 직업의길 + final rewards.

-- @brief   NPC Ryul: when QUEST_BEGINNER_PATH step 7 or completed, warp to return map; else intro dialogs, give exp 300 and 노란비서 20, set step 7, warp.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (률).
function NPC_555(me, npc)
    local quest = me:quest(QUEST_BEGINNER_PATH)
    if quest and (quest:step() == 7 or quest:completed()) then
        warp_to_return_map(me)
        return
    end

    ::NPC_555_COS001::
    local button = me:dialog(npc, "안녕하세요? 저는 률이라고 합니다. 초보자방의 마지막까지 오시느라 수고하셨습니다.", false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    ::NPC_555_COS002::
    button = me:dialog(npc, "이제는 이 좁은 초보자 방을 떠나, 드 넓은 바람의 나라 세계로 안내해 드리겠습니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_555_COS001
    end

    ::NPC_555_COS003::
    button = me:dialog(npc, "그 곳에서는 수천/수만의 사람들과 함께 게임을 즐길 수 있습니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_555_COS002
    end

    ::NPC_555_COS004::
    button = me:dialog(npc, "마지막으로 제가 노란비서 20개를 드리겠습니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_555_COS003
    end

    ::NPC_555_COS005::
    button = me:dialog(npc, "[노란비서]라는 아이템을 사용하시면 언제든지 [주막]으로 이동하시게 됩니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_555_COS004
    end

    ::NPC_555_COS006::
    button = me:dialog(npc, "단, 이것들은 1회용 이므로, 반드시 필요할 때만 사용하시기 바랍니다.", true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto NPC_555_COS005
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

    warp_to_return_map(me)
end
