-- npc: 여자죄수
local quest = require('lib.quest')
function NPC_393(me, npc)
    local q = me:quest(quest.QUEST_SILENCE)
    if q == nil then
        local btn = me:dialog(npc, "무슨 문제를 낼까...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local param = q:param() or ""
    local a, b, solved = param:match("^(%d+),(%d+),(%d+)$")
    if not a or not b or solved ~= "0" then
        local btn = me:dialog(npc, "무슨 문제를 낼까...", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    a = tonumber(a)
    b = tonumber(b)
    local expected = a + b

    local raw = me:input(npc, "털보죄수가 벌써 정답을 알아냈다구요? \\n ...그래요, 뭐라고 하던가요?", { top = "정답은", bottom = "라고 하더라구요.", maxlen = 8, prev = false })
    if raw == DIALOG_RESULT.QUIT then
        return
    end
    if raw == nil or raw == "" then
        return
    end

    local answer = tonumber(raw)
    if answer == expected then
        q:param(string.format("%d,%d,1", a, b))
        local btn = me:dialog(npc, "음... 문제가 너무 쉬웠던 것 같네요. 하지만 털보죄수에게 많은 문제를 풀어보라고 했었는데, 다른 문제들은 못풀었을 거에요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
    else
        local btn = me:dialog(npc, "그러면 그렇지, 역시 틀린 답이네요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
    end
end
