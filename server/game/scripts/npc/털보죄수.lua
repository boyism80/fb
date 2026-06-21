local quest = require('lib.quest')
function NPC_392(me, npc)
    local q = me:quest(quest.QUEST_SILENCE)

    if q ~= nil then
        local param = q:param() or ""
        local solved = param:match("^%d+,%d+,(%d+)$")
        if solved == "1" then
            me:mkitem("담배", 1)
            q:param("")
            local btn = me:dialog(npc, "이야, 고맙네 고마워! 자네 덕분에 여자죄수의 큰 코를 납작하게 눌러 버릴 수 있었다네!", false, true)
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            return
        end
    end

    if q == nil or q:param() == "" or not q:param():match("^%d+,%d+,0$") then
        local btn = me:dialog(npc, "자네, 혹시 문제풀이를 잘 하는가? \\n 오랫동안 감옥에 있다보니 저 건너편에 앉아있는 여 죄수와 계산문제를 주고 받는게 내 삶의 낙이라네.", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "그런데 이번 문제는 너무 어려워서 벌서 몇 시간째 고민을 하고 있다네. 조금만 더 늦으면 여자죄수에게 무시를 당할텐데 말이야...", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "그래서 말인데 자네, 혹시 나 대신 문제를 풀어 여자죄수에게 전해주지 않겠나? ", false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        local a = math.random(1, 1000)
        local b = math.random(1, 1000)
        if q == nil then
            q = me:start_quest(quest.QUEST_SILENCE)
            if q == nil then
                return
            end
        end
        if q then
            q:param(string.format("%d,%d,0", a, b))
        end
        btn = me:dialog(npc, string.format("자, 내가 풀어야할 문제는 \\n [ %d ] + [ %d ] = [ ? ] \\n 이라네. [?]의 답을 생각해 건너편 죄수에게 전해주게.", a, b), false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        return
    end

    local param = q:param() or ""
    local a, b = param:match("^(%d+),(%d+),0$")
    if a and b then
        a = tonumber(a)
        b = tonumber(b)
        local btn = me:dialog(npc, string.format("자, 내가 풀어야할 문제는 \\n [ %d ] + [ %d ] = [ ? ] \\n 이라네. [?]의 답을 생각해 건너편 죄수에게 전해주게.", a, b), false, true)
        if btn == DIALOG_RESULT.QUIT then
            return
        end
    end
end
