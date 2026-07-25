-- npc: 흉노장로2
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local q = me:quest(quest.QUEST_JINHWANG)
        if q == nil or q:step() ~= 3 then
            return
        end

        if not me:has_items("천상복숭아", 1) then
            me:dialog(npc, "퀘스트 오류입니다.\n\n운영자에게 문의하세요.", { prev = false, next = true })
            return
        end

        ::NPC_545_0000::
        local button = me:dialog(npc, "이.. 이것은? 천상복숭아?? 자네 이걸 어디서 구했는가. 천도복숭아보다 구하기 훨씬 어려운것을.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_545_0001::
        local sel, list_btn = me:list(npc, "무리한 부탁인건 아네만 이걸 나에게 넘겨주게. 내 아들의 병을 고칠수 있는것 천상복숭아 뿐이네. 무엇이든 들어줄테니 제발...", {
            "좋소! 대신 진황보검을 넘겨주시오!",
            "미안하지만 이것만은 안되오.",
        }, { prev = true })
        if list_btn == DIALOG_RESULT.QUIT then
            return
        end
        if list_btn == DIALOG_RESULT.PREV then
            goto NPC_545_0000
        end
        if sel == nil or sel ~= 1 then
            return
        end

        ::NPC_545_0002::
        button = me:dialog(npc, "좋소. 아들의 생명이 달린 문제인데 어떤 물건인들 무슨 소용이 있겠소. 자 진랑검이요. 진황보검이 진랑검과 혈황검 이 두개로 이루어져있는건 알고 있겠지..", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_545_0001
        end

        ::NPC_545_0003::
        button = me:dialog(npc, "내가 가지고 있는것은 진랑검 뿐이네. 혈황검의 행방은 나도 모르오.. 누구에게 처음 들었는지 모르지만 그라면 혈황검에 대해서도 알지 않겠소?", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_545_0002
        end

        ::NPC_545_0004::
        button = me:dialog(npc, "어찌됐든 아들의 목숨을 구할 수 있게 되어서 너무 감사하오. 그럼 이만..", { prev = false, next = false })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_545_0003
        end

        local code = me:exchange(
            { ['item'] = { ["천상복숭아"] = 1 } },
            { ['item'] = { ["진랑검"] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, "천상복숭아가 없지 않은가.", { prev = false, next = false })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 줄 수 없네.", { prev = false, next = false })
            return
        end

        me:push_achievement(511, "진황보검을 찾아서 (진랑검을 찾았다)", 7, 20)
        q:step(4)
    end
}
