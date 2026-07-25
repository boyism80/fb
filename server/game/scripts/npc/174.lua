-- npc: 진백랑
local quest = require('lib.quest')
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local diary_names = {
            '선장의일기1', '선장의일기2', '선장의일기3', '선장의일기4', '선장의일기5',
            '선장의일기6', '선장의일기7', '선장의일기8', '선장의일기9', '선장의일기겉표지',
        }
        local q = me:quest(quest.QUEST_LIGHTHOUSE)
        local btn

        if q == nil then
            ::NPC_174_0001::
            btn = me:dialog(npc, '대륙의 기운이 느껴지는군. 자네라면 내 억울함을 풀어줄 수 있을지도 모르지.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel, lb = me:list(npc, '난 오래전 이 섬에서 죽은 중국의 선장이다. 날 좀 도와주겠나?', { '네, 도와드리지요.', '아니오.. 싫습니다.' }, { prev = false })
            if lb == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel ~= 1 then
                return
            end
            ::NPC_174_0003::
            btn = me:dialog(npc, '나는 원래 서복이란 자를 쫓아 이곳에 당도했다. 결국 서복은 없었지만 너무나 아름다운 이 섬에서 얼마간 머물기로 마음먹었지. 그 결과 나는 엄청난 비밀을 알게 되었고, 그 대가로 이렇게 원혼이 되었다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_174_0004::
            btn = me:dialog(npc, '죽었다는 사실은 억울하지 않아. 다만, 내가 이 섬에 대해 기록한 일지가 세상에 알려지지 않은 것이 슬플뿐이야. 이미 섬 이곳저곳으로 흩어져 버렸지만.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_174_0003
            end
            btn = me:dialog(npc, '전부 삼만장 정도 되는데, 아홉장만 모아도 굉장히 충실한 내용이야. 아홉장과 겉표지를 모아서 내게 가져다주면 좋겠군.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            q = me:start_quest(quest.QUEST_LIGHTHOUSE)
            if q == nil then
                return
            end
            q:step(1)
            me:push_achievement(32, '진백랑의 부탁을 들어주자. [1/6]', 7, 1)
            return
        end

        if q:step() == 0 then
            ::NPC_174_0010::
            btn = me:dialog(npc, '대륙의 기운이 느껴지는군. 자네라면 내 억울함을 풀어줄 수 있을지도 모르지.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local sel, lb = me:list(npc, '난 오래전 이 섬에서 죽은 중국의 선장이다. 날 좀 도와주겠나?', { '네, 도와드리지요.', '아니오.. 싫습니다.' }, { prev = false })
            if lb == DIALOG_RESULT.QUIT then
                return
            end
            if sel == nil or sel ~= 1 then
                return
            end
            ::NPC_174_0011::
            btn = me:dialog(npc, '나는 원래 서복이란 자를 쫓아 이곳에 당도했다. 결국 서복은 없었지만 너무나 아름다운 이 섬에서 얼마간 머물기로 마음먹었지. 그 결과 나는 엄청난 비밀을 알게 되었고, 그 대가로 이렇게 원혼이 되었다.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_174_0012::
            btn = me:dialog(npc, '죽었다는 사실은 억울하지 않아. 다만, 내가 이 섬에 대해 기록한 일지가 세상에 알려지지 않은 것이 슬플뿐이야. 이미 섬 이곳저곳으로 흩어져 버렸지만.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_174_0011
            end
            btn = me:dialog(npc, '전부 삼만장 정도 되는데, 아홉장만 모아도 굉장히 충실한 내용이야. 아홉장과 겉표지를 모아서 내게 가져다주면 좋겠군.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            q = me:start_quest(quest.QUEST_LIGHTHOUSE)
            if q == nil then
                return
            end
            q:step(1)
            me:push_achievement(32, '진백랑의 부탁을 들어주자. [1/6]', 7, 1)
            return
        end

        if q:completed() then
            me:dialog(npc, '......', { prev = true, next = true })
            return
        end

        do
            local give_table = {}
            for _, name in ipairs(diary_names) do
                give_table[name] = 1
            end
            if not me:has_items(give_table) then
                me:dialog(npc, '아직 일기 아홉장과 겉표지를 구하지 못했나보군..', { prev = false, next = false })
                return
            end
            ::NPC_174_0020::
            btn = me:dialog(npc, '일기장을 정리해오겠네 좀 기다려주게나', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            ::NPC_174_0021::
            btn = me:dialog(npc, '수고했네. 여기 일기장이 있어. 이 섬을 여행하는데 있어 좋은 도움이 되었으면 더 바랄 나위가 없을 것 같네.', { prev = true, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            if btn == DIALOG_RESULT.PREV then
                goto NPC_174_0020
            end
            btn = me:dialog(npc, '그리고 이건, 내가 생전에 쓰던 검이네. 등대와 같은 빛을 낸다하여 등대빛의 검이라 불리우지. 괜찮다면 꼭 좋은 일에 써주길 바라네.', { prev = false, next = true })
            if btn == DIALOG_RESULT.QUIT then
                return
            end
            local code = me:exchange(
                { ['item'] = give_table },
                { ['item'] = { ['선장의일기장'] = 1, ['등대빛의검'] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '아직 일기 아홉장과 겉표지를 구하지 못했나보군..', { prev = false, next = true })
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 보상을 줄 수 없네.', { prev = false, next = true })
                return
            end
            q:complete()
            me:push_achievement(32, '등대빛의검을 받다!', 6, 1)
            return
        end
    end
}
