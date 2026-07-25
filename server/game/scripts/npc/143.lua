-- npc: 중국공주
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local q = me:quest(quest.QUEST_PRINCESS_RING)

        if q == nil then
            ::NPC_143_0001::
            local btn = me:dialog(npc, '제가 강에서 배를 타다가 반지를 빠뜨리고 말았어요. 그 반지는 옆나라의 제 정혼자인 왕자가 준 반지라 잃어버리면 그 나라와의 관계까지 위험해질수 있어요.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_143_0002::
            btn = me:dialog(npc, '찾으러 여러분이 들어갔지만 아직도 나오신 분이 없네요. 아무래도 평범한 강이 아닌거 같아요.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_143_0001
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_143_0003::
            local sel = me:list(npc, '제발 부탁드려요. 반지를 찾아주세요.', { '걱정마세요. 제가 꼭 찾아드리죠.', '흠. 전 물을 싫어해서요. 죄송하네요.' }, { prev = false })
            if sel == nil or sel == 2 then
                if sel == 2 then
                    me:dialog(npc, '그러시다면야.. 이를 어쩐담...', { prev = false, next = true })
                end
                return
            end
            btn = me:dialog(npc, '정말 고마워요. 그럼 믿고 기다리죠.', { prev = false, next = true })
            local q = me:start_quest(quest.QUEST_PRINCESS_RING)
            if q == nil then
                me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
                return
            end
            me:push_achievement(3, '중국공주의 잃어버린 반지를 되찾자.', 7, 1)
            return
        end

        if q:completed() then
            me:dialog(npc, '저번엔 정말로 감사했습니다.', { prev = false, next = true })
            return
        end

        if not me:has_items('공주의반지', 1) then
            me:dialog(npc, '아직 제 반지가 없으신거같은데..', { prev = false, next = true })
            return
        end
        local btn = me:dialog(npc, '정말 감사해요. 정말... 이 은혜는 잊지 못할꺼에요. 이건 약소하지만 제가 드리는 선물입니다.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = { ['공주의반지'] = 1 } },
            { ['item'] = { ['청옥반지'] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 제 반지가 없으신거같은데..', { prev = false, next = true })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 청옥반지를 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        q:complete()
        me:push_achievement(3, '잃어버린 공주의 반지를 되찾아 주다.', 7, 1)
    end
}
