-- npc: 좌선녀
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    ON_CLICK = function(me, npc)
        local ACHIEVEMENT_WATER = 18
        local MIN_LEVEL = 36
        local q = me:quest(quest.QUEST_WATER_RING)
        local btn, sel

        if me:level() < MIN_LEVEL then
            me:dialog(npc, '용왕님의 건강이 많이 나빠지셔서 어떡하면 좋을까..', { prev = false, next = true })
            return
        end

        if q == nil then
            ::NPC_76_COS001::
            btn = me:dialog(npc, '안녕하세요? ' .. me:name() .. '님, 전 용왕님의 식사를 담당하고 있는 좌선녀라해요', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_76_COS002::
            btn = me:dialog(npc, '근래 용왕님께서 여러 어려운 일을 처리하시느라 몸이 상당히 나빠지셨어요', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_76_COS001
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_76_COS003::
            btn = me:dialog(npc, '그래서 제가 용왕님의 건강에 도움이 될 천상보강탕을 해 드릴려고 하는데 재료가 부족하네요.', { prev = true, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_76_COS002
            end
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_76_COS004::
            sel = me:list(npc, '제가 부탁하는 재료를 구해주실 수 있나요?', { '아.물론입니다.', '죄송하지만 바빠서...' })
            if sel == nil or sel ~= 1 then
                if sel == 2 then
                    me:dialog(npc, '흠..그러신가요..', { prev = false, next = true })
                end
                return
            end
            if q == nil then
                q = me:start_quest(quest.QUEST_WATER_RING)
                if q == nil then
                    me:dialog(npc, '퀘스트 시작 실패', { prev = false, next = true })
                    return
                end
            end
            q:step(1)
            me:push_achievement(ACHIEVEMENT_WATER, '좌선녀의 부탁을 들어주자.', 7, 1)
            ::NPC_76_COS005::
            btn = me:dialog(npc, me:name() .. '님 정말 감사합니다. 부탁드릴 재료는 [게등껍질][게집게][문어다리][해마꼬리]랍니다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            btn = me:dialog(npc, '그럼 부탁드리겠습니다.', { prev = false, next = true })
            if btn == DIALOG_RESULT.PREV then
                goto NPC_76_COS005
            end
            return
        end

        if q:completed() then
            return
        end

        local materials = {['게등껍질'] = 1, ['게집게'] = 1, ['문어다리'] = 1, ['해마꼬리'] = 1}
        if not me:has_items(materials) then
            me:dialog(npc, '아직 재료를 다 모으지 못하신 것 같군요.. 재료는 [게등껍질][게집게][문어다리][해마꼬리]랍니다.', { prev = false, next = true })
            return
        end

        ::NPC_76_COS007::
        btn = me:dialog(npc, '어머, 재료를 벌써 다 모아 오셨군요. 정말 감사드려요.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        local code = me:exchange(
            { ['item'] = materials },
            { ['item'] = { ['인어반지'] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, '아직 재료를 다 모으지 못하신 것 같군요.. 재료는 [게등껍질][게집게][문어다리][해마꼬리]랍니다.', { prev = false, next = true })
            return
        elseif code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 인어반지를 받을 수 없습니다.', { prev = false, next = true })
            return
        end
        q:complete()
        me:push_achievement(ACHIEVEMENT_WATER, '좌선녀의 부탁을 들어주었다.', 7, 1)
        btn = me:dialog(npc, '이 인어반지는 제 성의에요. 예전 인어장군님께 선물을 받은 것인데 공좌님의 성의에 꼭 보답하고 싶어서요.', { prev = false, next = true })
        if btn == DIALOG_RESULT.PREV then
            goto NPC_76_COS007
        end
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        me:dialog(npc, '공좌님 정말 감사드려요.', { prev = false, next = true })
    end
}
