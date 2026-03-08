-- @note Trash: 0_초보지원\엔피씨.txt "말도". QUEST_BEGINNER_PATH: intro dialogs and "안녕하세요" repeat; step 2 gives exp 300 and step(3); step >= 3 next-room message.

-- @brief   NPC Maldo: speaking tutorial (ENTER key, repeat phrase). Uses QUEST_BEGINNER_PATH; step 2 gives exp 300.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (말도).
function NPC_445(me, npc)
    local quest = me:quest(QUEST_BEGINNER_PATH)

    if quest == nil then
        me:dialog(npc, string.format("%s님 안녕하세요? 저는 말도라고 합니다. 말하기 도우미가 떠오른다면 그건 착각일 뿐입니다.", me:name()), false, true)
        local btn = me:dialog(npc, "저는 말하는 방법을 알려드리겠습니다.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "키보드에 있는 [ENTER(엔터)]키를 누르시고, 제가 하는 말을 따라 해 보세요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "안녕하세요", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        quest = me:start_quest(QUEST_BEGINNER_PATH)
        if quest == nil then
            me:dialog(npc, "퀘스트를 시작할 수 없습니다.", false, false)
            return
        end
        if quest then
            quest:step(1)
        end
        return
    end

    if quest:completed() then
        me:dialog(npc, "제가 가르쳐드릴게 이거밖에 없어서 미안하네요. 말하기는 [ENTER]요. 흥... 이제 다음 방으로 가보세요.", false, false)
        return
    end

    local step = quest:step()
    if step == 1 then
        local btn = me:dialog(npc, "키보드에 있는 [ENTER(엔터)]키를 누르시고, 제가 하는 말을 따라 해 보세요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "안녕하세요", false, true)
        return
    end

    if step == 2 then
        me:dialog(npc, "잘하셨어요. 딱히 아이템을 드리지는 않겠지만, 선물로 경험치를 300 드릴게요.", false, true)
        me:exp(me:exp() + 300)
        quest:step(3)
        return
    end

    -- step >= 3
    me:dialog(npc, "제가 가르쳐드릴게 이거밖에 없어서 미안하네요. 말하기는 [ENTER]요. 흥... 이제 다음 방으로 가보세요.", false, false)
end
