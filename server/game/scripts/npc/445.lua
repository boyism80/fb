-- npc: 말도
local quest = require('lib.quest')
function NPC_445(me, npc)
    local q = me:quest(quest.QUEST_BEGINNER_PATH)

    if q == nil then
        me:dialog(npc, string.format("%s님 안녕하세요? 저는 말도라고 합니다. 말하기 도우미가 떠오른다면 그건 착각일 뿐입니다.", me:name()), { prev = false, next = true })
        local btn = me:dialog(npc, "저는 말하는 방법을 알려드리겠습니다.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "키보드에 있는 [ENTER(엔터)]키를 누르시고, 제가 하는 말을 따라 해 보세요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        btn = me:dialog(npc, "안녕하세요", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        q = me:start_quest(quest.QUEST_BEGINNER_PATH)
        if q == nil then
            me:dialog(npc, "퀘스트를 시작할 수 없습니다.", { prev = false, next = false })
            return
        end
        if q then
            q:step(1)
        end
        return
    end

    if q:completed() then
        me:dialog(npc, "제가 가르쳐드릴게 이거밖에 없어서 미안하네요. 말하기는 [ENTER]요. 흥... 이제 다음 방으로 가보세요.", { prev = false, next = false })
        return
    end

    local step = q:step()
    if step == 1 then
        local btn = me:dialog(npc, "키보드에 있는 [ENTER(엔터)]키를 누르시고, 제가 하는 말을 따라 해 보세요.", { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, "안녕하세요", { prev = false, next = true })
        return
    end

    if step == 2 then
        me:dialog(npc, "잘하셨어요. 딱히 아이템을 드리지는 않겠지만, 선물로 경험치를 300 드릴게요.", { prev = false, next = true })
        me:exp(me:exp() + 300)
        q:step(3)
        return
    end

    me:dialog(npc, "제가 가르쳐드릴게 이거밖에 없어서 미안하네요. 말하기는 [ENTER]요. 흥... 이제 다음 방으로 가보세요.", { prev = false, next = false })
end
