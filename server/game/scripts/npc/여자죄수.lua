-- @note Trash: 2_이벤트\묵언수행.txt "여자죄수". Global $silence1,$silence2,$silence3,$silence4 -> per-character QUEST_SILENCE param "a,b,solved". Input answer; if a+b correct set solved=1; else wrong message.

---@brief   NPC 여자죄수: accepts answer for 털보죄수's problem (a+b). Correct -> set solved so 털보죄수 gives 담배. Uses QUEST_SILENCE param "a,b,solved".
---@param[in] me   The character.
---@param[in] npc  The NPC entity.
function NPC_393(me, npc)
    local quest = me:quest(QUEST_SILENCE)
    if quest == nil then
        local btn = me:dialog(npc, "무슨 문제를 낼까...", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local param = quest:param() or ""
    local a, b, solved = param:match("^(%d+),(%d+),(%d+)$")
    if not a or not b or solved ~= "0" then
        local btn = me:dialog(npc, "무슨 문제를 낼까...", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    a = tonumber(a)
    b = tonumber(b)
    local expected = a + b

    local raw = me:input(npc, "털보죄수가 벌써 정답을 알아냈다구요? \\n ...그래요, 뭐라고 하던가요?", "정답은", "라고 하더라구요.", 8, false)
    if raw == DIALOG_RESULT.QUIT then
        return
    end
    if raw == nil or raw == "" then
        return
    end

    local answer = tonumber(raw)
    if answer == expected then
        quest:param(string.format("%d,%d,1", a, b))
        local btn = me:dialog(npc, "음... 문제가 너무 쉬웠던 것 같네요. 하지만 털보죄수에게 많은 문제를 풀어보라고 했었는데, 다른 문제들은 못풀었을 거에요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
    else
        local btn = me:dialog(npc, "그러면 그렇지, 역시 틀린 답이네요.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
    end
end
