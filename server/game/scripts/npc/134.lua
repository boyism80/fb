-- npc: 돌돌이
local quest = require('lib.quest')
function NPC_134(me, npc)
    local q = me:quest(quest.QUEST_DETECTIVE)
    if q == nil or q:completed() then
        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
        return
    end
    local step = q:step()

    if step == 1 or step == 2 then
        local sel, list_btn = me:list(npc, "우와! 그거 수사관 명찰이잖아요!\n도대체 무슨 일이에요?!", {
            "나 좀 도와주겠니?",
            "나중에 다 이야기해주마.",
            "아. 오늘은 요리 이야기를 좀...",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            local btn = me:dialog(npc, "네! 뭐든지 도와드릴게요!", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, "오호라... 과연, 문화재 유출범을 찾고 계신다는 거군요. 알겠어요, 있는 힘껏 정보를 수집해볼게요!", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            q:step(3)
            me:dialog(npc, "잘 부탁드립니다, 수사관 나리... 헤헤헤.", { prev = false, next = false })
        elseif sel == 2 then
            me:dialog(npc, "여기서 기다리고 있을게요!", { prev = false, next = false })
        else
            me:dialog(npc, "저 요리같은거 할 줄 몰라요~!", { prev = false, next = false })
        end
        return
    end

    if step == 8 then
        local sel, list_btn = me:list(npc, "안녕하세요?", {
            "전에 부탁한 건 어떻게 됐니?",
            "그래, 열심히 정보를 모아다오.",
            "아, 오늘은 요리 이야기를 좀.",
        }, { prev = false })
        if list_btn == DIALOG_RESULT.QUIT or sel == nil then
            return
        end
        if sel == 1 then
            local btn = me:dialog(npc, "그러고보니, 진진 아저씨, 며칠전에 갑자기 돈이 생겨서 큰 집을 샀다고해요.", { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            q:step(9)
            me:dialog(npc, "홀어머니 모시느라 고생하시더니, 다행이지 뭐예요. 어떻게 그런 큰 돈을 모았는지는 아무도 모르지만요.", { prev = false, next = false })
        elseif sel == 2 then
            me:dialog(npc, "저 돌돌이를 믿어주세요~!", { prev = false, next = false })
        else
            me:dialog(npc, "저 요리같은거 잘 못해요~!", { prev = false, next = false })
        end
        return
    end

    me:dialog(npc, "준비중입니다.", { prev = false, next = false })
end