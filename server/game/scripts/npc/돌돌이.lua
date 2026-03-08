-- @note Trash: 1_환상의섬\일반.txt "돌돌이". QUEST_DETECTIVE: step 1 or 2 → list "나 좀 도와주겠니?" → choice 1 → 3 dialogs → step(3). Step 8 → list "전에 부탁한 건 어떻게 됐니?" → choice 1 → 2 dialogs → step(9). Else $doldol_gosool etc omitted → placeholder.

--- @brief NPC 돌돌이: Detective quest. Step 1,2 → 3; step 8 → 9.
--- @param me   character
--- @param npc  NPC entity
function NPC_134(me, npc)
    local quest = me:quest(QUEST_DETECTIVE)
    if quest == nil or quest:completed() then
        me:dialog(npc, "준비중입니다.", false, false)
        return
    end
    local step = quest:step()

    if step == 1 or step == 2 then
        local sel, list_btn = me:list(npc, "우와! 그거 수사관 명찰이잖아요!\n도대체 무슨 일이에요?!", {
            "나 좀 도와주겠니?",
            "나중에 다 이야기해주마.",
            "아. 오늘은 요리 이야기를 좀...",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            local btn = me:dialog(npc, "네! 뭐든지 도와드릴게요!", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "오호라... 과연, 문화재 유출범을 찾고 계신다는 거군요. 알겠어요, 있는 힘껏 정보를 수집해볼게요!", true, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                return
            end
            quest:step(3)
            me:dialog(npc, "잘 부탁드립니다, 수사관 나리... 헤헤헤.", true, false)
        elseif sel == 1 then
            me:dialog(npc, "여기서 기다리고 있을게요!", false, false)
        else
            me:dialog(npc, "저 요리같은거 할 줄 몰라요~!", false, false)
        end
        return
    end

    if step == 8 then
        local sel, list_btn = me:list(npc, "안녕하세요?", {
            "전에 부탁한 건 어떻게 됐니?",
            "그래, 열심히 정보를 모아다오.",
            "아, 오늘은 요리 이야기를 좀.",
        }, false)
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 0 then
            local btn = me:dialog(npc, "그러고보니, 진진 아저씨, 며칠전에 갑자기 돈이 생겨서 큰 집을 샀다고해요.", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            quest:step(9)
            me:dialog(npc, "홀어머니 모시느라 고생하시더니, 다행이지 뭐예요. 어떻게 그런 큰 돈을 모았는지는 아무도 모르지만요.", true, false)
        elseif sel == 1 then
            me:dialog(npc, "저 돌돌이를 믿어주세요~!", false, false)
        else
            me:dialog(npc, "저 요리같은거 잘 못해요~!", false, false)
        end
        return
    end

    me:dialog(npc, "준비중입니다.", false, false)
end
