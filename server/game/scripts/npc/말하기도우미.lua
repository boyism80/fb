-- @note Trash: 1_초보자의길\초보자도우미.txt "말하기도우미". QUEST_BEGINNER_PATH: quest==nil → intro+start_quest+step(1), step 1→2 (도톨, exp 100), step 2→3 (no 도톨: exp 100), step≥3 사냥도우미 안내.

-- @brief   NPC speaking helper: QUEST_BEGINNER_PATH — not started (quest==nil): intro then start_quest and step(1); step 1→2 (도톨, exp), step 2→3; step≥3 hint to 사냥도우미.
-- @param[in]  me   The character talking to the NPC.
-- @param[in]  npc  The NPC entity (말하기도우미).
function NPC_276(me, npc)
    local quest = me:quest(QUEST_BEGINNER_PATH)

    if quest == nil then
        me:dialog(npc, string.format("%s님, 안녕하세요?\n저는 말하기도우미 입니다.", me:name()), false, true)
        me:dialog(npc, "저는 바람의나라에서 말을 하는 다양한 방법을 알려드릴거에요.", false, true)
        me:dialog(npc, "우선 일반적인 말을 하기\n 위해서는 [Enter]를 누르시거나\n[']키를 누른 뒤 하고싶은\n 말을 적으시면 된답니다.", false, true)
        me:dialog(npc, "물론 할 말을 마친 뒤에는 같은 키를 눌러 마무리를 하시면 됩니다.", false, true)
        me:dialog(npc, "그리고 외치기라는 방법도\n있는데, 외치기는 [Shift]+[!]를\n 눌러 할 수 있답니다.\n\n레벨이 2가 넘어야 할 수 있으니 나중에 해보세요.", false, true)
        me:dialog(npc, "그럼 먼저 가르쳐 드린 대로\n[']키나 [Enter]를 눌러\n '이렇게요?'라고 말 해 보세요.", false, true)
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
        me:dialog(npc, "이제 제가 가르쳐 드릴 것은 모두 가르쳐 드린 것 같네요.\n다음 방에서 사냥도우미가 기다리고 있으니 어서 가보세요.", false, true)
        me:dialog(npc, "보다 넓은 세계에서 잘 해내리라 믿어요.\n안녕히가세요.", false, false)
        return
    end

    local step = quest:step()
    if step >= 3 then
        me:dialog(npc, "이제 제가 가르쳐 드릴 것은 모두 가르쳐 드린 것 같네요.\n다음 방에서 사냥도우미가 기다리고 있으니 어서 가보세요.", false, true)
        me:dialog(npc, "보다 넓은 세계에서 잘 해내리라 믿어요.\n안녕히가세요.", false, false)
        return
    end

    if step == 1 then
        me:dialog(npc, "잘 하셨습니다. 상으로 도톨을 드릴게요.", false, true)
        me:exp(me:exp() + 100)
        if me:mkitem("도톨", 2) == nil then
            me:dialog(npc, "소지품이 가득 차서 도톨을 줄 수 없습니다.", false, false)
            return
        end
        quest:step(2)
        me:dialog(npc, "제가 드린 도톨을 확인하시려면\n[i]를 눌러 아이템창을 열어보시면 된답니다.\n그리고 [e]키를 누른 뒤 도톨이 있는 알파벳을 쓰면 됩니다.\n\n아마 제가 드린 도톨이 a에 있을거에요.", false, true)
        me:dialog(npc, "그럼, 제가 알려드린 방법을 사용하여 [e, a]를 눌러 도톨을 모두 먹어보세요. 다 하셨으면 저를 클릭 해 주세요.", false, false)
        return
    end

    -- step == 2
    if me:has_items("도톨", 1) then
        me:dialog(npc, "그럼, 제가 알려드린 방법을 사용하여 [e, a]를 눌러 도톨을 모두 먹어보세요. 다 하셨으면 저를 클릭 해 주세요.", false, false)
        return
    end

    quest:step(3)
    me:exp(me:exp() + 100)
    me:dialog(npc, "정말 잘 하셨어요!.", false, true)
    me:dialog(npc, "이제 제가 가르쳐 드릴 것은 모두 가르쳐 드린 것 같네요.\n다음 방에서 사냥도우미가 기다리고 있으니 어서 가보세요.", false, true)
    me:dialog(npc, "보다 넓은 세계에서 잘 해내리라 믿어요.\n안녕히가세요.", false, false)
end
