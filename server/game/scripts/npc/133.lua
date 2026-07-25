-- npc: 돌순이
local quest = require('lib.quest')

return {
    ON_CLICK = function(me, npc)
        local q_jingo = me:quest(quest.QUEST_JINGOGYUN)
        if q_jingo and q_jingo:step() == 5 then
            ::NPC_133_0001::
            local b = me:dialog(npc, "응? 영력구슬이요? 음, 다른 사람한테 줬는데... 아쉽게 되었네요. 헤헤헤.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_133_0010::
            b = me:dialog(npc, "누가 가져갔냐고요? 사실 두개 갖고 있었는데, 하나는 찬찬 아저씨에게 주고 비장의 도시락을 받았고", { prev = true, next = true })
            if b == DIALOG_RESULT.PREV then
                goto NPC_133_0001
            end
            if b == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_133_0011::
            b = me:dialog(npc, "나머지 하나는 독립군 인성초들에게 줬어요. 아마 이름이 오른이였을거에요. 아니다 수수였던가.", { prev = true, next = true })
            if b == DIALOG_RESULT.PREV then
                goto NPC_133_0010
            end
            if b == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_133_0012::
            b = me:dialog(npc, "아, 정말 다들 똑같이 생겨서 잘은 모르겠지만, 아무튼 인성초들이 가지고 있을거에요.", { prev = false, next = true })
            if b == DIALOG_RESULT.QUIT then
                return
            end
            q_jingo:step(6)
            return
        end

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
                q:step(2)
                me:dialog(npc, "잘 부탁드립니다, 수사관 나리... 헤헤헤.", { prev = false, next = false })
            elseif sel == 2 then
                me:dialog(npc, "여기서 기다리고 있을게요!", { prev = false, next = false })
            else
                me:dialog(npc, "저 요리같은거 할 줄 몰라요~!", { prev = false, next = false })
            end
            return
        end

        if step == 11 then
            local sel, list_btn = me:list(npc, "수사관님! 수사는 잘 되가세요?", {
                "새롭게 얻은 정보는 있니?",
            }, { prev = false })
            if list_btn == DIALOG_RESULT.QUIT or sel == nil then
                return
            end
            if sel == 1 then
                local btn = me:dialog(npc, "친구가 그러는데, 정령사가 굉장히 화난 모양이에요.", { prev = false, next = true })
                if btn == DIALOG_RESULT.QUIT then
                    return
                end
                q:step(12)
                me:dialog(npc, "토템이 이 땅에서 점점 없어져서 정령들이 화가 났다나봐요, 히히히.", { prev = false, next = false })
            end
            return
        end

        me:dialog(npc, "준비중입니다.", { prev = false, next = false })
    end
}
