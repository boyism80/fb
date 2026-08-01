-- npc: 도삭산대신선
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_DOJAECHUN)
        if q == nil then
            return
        end
        if q:completed() then
            return
        end
        if q:step() ~= 1 then
            return
        end

        if not me:has_items("신선의책", 1) then
            return
        end

        ::NPC_368_0000::
        local button = me:dialog(npc, "자, 그럼 이 책의 마법을 시전하겠네. 신선의책에 담긴 힘을 봉인을 풀어 다시 원위치로 돌아가게 하는 것일세.", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_368_0001::
        button = me:dialog(npc, "'현명한 옛 선인들의 지혜가 담겨 있는 신선의책이여, 신선의 이름으로 명하노니 도삭산의 모든 질서를 바로잡고 원래의 모습을 되찾도록 하여라!'", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_368_0000
        end

        ::NPC_368_0002::
        button = me:dialog(npc, "자네가 구해온 이 신선의책으로 인해서 이제 도삭산은 점점 질서를 찾아갈 것이네. 내 '도삭산의서'를 줄테니 소중히 간직하고 잘 쓰도록 하게나. 이 책은 도삭산의황금봉, 수정해골등과 함께 어딘가에 쓸 곳이 있을 것이네.", { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end
        if button == DIALOG_RESULT.PREV then
            goto NPC_368_0001
        end

        button = me:dialog(npc, "그럼 다음에 기회가 되면 또 보세..", { prev = false, next = true })
        if button == DIALOG_RESULT.QUIT then
            return
        end

        local code = me:exchange(
            { ['item'] = { ["신선의책"] = 1 } },
            { ['item'] = { ["도삭산의서"] = 1 } }
        )
        if code == enum.exchange_result.LACK_COST then
            me:dialog(npc, "신선의책이 없지 않은가.", { prev = false, next = false })
            return
        end
        if code == enum.exchange_result.LACK_CAPACITY then
            me:dialog(npc, "소지품이 가득 차서 줄 수 없네.", { prev = false, next = false })
            return
        end

        local map = name2map("도삭산900층주막")
        if map then
            me:map(map, { 14, 14 })
        end

        me:push_achievement(47, "도삭산 900층 퀘스트 완료", 6, 25)
        q:complete()
    end
}
