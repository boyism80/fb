-- npc: 영채
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local selected, button = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {'방송쿠폰', '웨딩드레스 수선'})
        if button == DIALOG_RESULT.QUIT then
            return
        end

        if selected == 1 then
            button = me:dialog(npc, '방송쿠폰은 나만이 팔고 있지. 호호홋...', { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end

            selected, button = me:list(npc, '방송쿠폰의 가격은 100 전 이라네. 방송쿠폰을 살텐가?', {'예. 구입하고 싶습니다.', '아니오. 필요 없습니다.'})
            if button == DIALOG_RESULT.QUIT then
                return
            end

            if selected == 1 then
                if me:level() < 21 then
                    me:dialog(npc, '21레벨 구매가 가능합니다.', { prev = false, next = true })
                    return
                end

                local money = me:money()
                if money < 100 then
                    me:dialog(npc, '방송쿠폰의 가격은 100 전 이라네. 돈이 부족한 것 아닌가?', { prev = false, next = true })
                    return
                end

                local item = me:mkitem('방송쿠폰', 1)
                if item == nil then
                    me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', { prev = false, next = true })
                    return
                end
                me:money(money - 100)
                me:dialog(npc, '운이 좋군. 잘 쓰시게...', { prev = false, next = true })
            end
            return
        end

        if selected == 2 then
            button = me:dialog(npc, '웨딩드레스를 수선하고 싶다고? 그렇다면 웨딩드레스1,2,3,4,5,6,7 과 말린지네 7개를 가져온다면 수선해주도록 하지.', { prev = false, next = true })
            if button == DIALOG_RESULT.QUIT then
                return
            end

            selected, button = me:list(npc, '웨딩드레스를 수선하겠는가?', {'예', '아뇨'})
            if button == DIALOG_RESULT.QUIT then
                return
            end

            if selected ~= 1 then
                return
            end

            local items = me:items()
            local count_by_name = {}
            if items ~= nil then
                for _, item in pairs(items) do
                    local name = item:model():name()
                    count_by_name[name] = (count_by_name[name] or 0) + item:count()
                end
            end

            if (count_by_name['말린지네'] or 0) < 7 then
                me:dialog(npc, '재료가 부족한데? 재료를 다 가져와야 수선해줄 수 있어.', { prev = false, next = true })
                return
            end

            local dress_names = {'웨딩드레스1', '웨딩드레스2', '웨딩드레스3', '웨딩드레스4', '웨딩드레스5', '웨딩드레스6', '웨딩드레스7'}
            for _, name in ipairs(dress_names) do
                if (count_by_name[name] or 0) < 1 then
                    me:dialog(npc, '재료가 부족한데? 재료를 다 가져와야 수선해줄 수 있어.', { prev = false, next = true })
                    return
                end
            end

            local cost = { ['말린지네'] = 7 }
            for _, name in ipairs(dress_names) do
                cost[name] = 1
            end
            local rand = math.random(1, 4)
            local result_name = '웨딩드레스' .. (rand + 7)
            local code = me:exchange(
                { ['item'] = cost },
                { ['item'] = { [result_name] = 1 } }
            )
            if code == enum.EXCHANGE_RESULT.LACK_COST then
                me:dialog(npc, '재료가 부족한데? 재료를 다 가져와야 수선해줄 수 있어.', { prev = false, next = true })
                return
            end
            if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
                me:dialog(npc, '소지품이 가득 차서 줄 수가 없네.', { prev = false, next = true })
                return
            end
            me:dialog(npc, '여기 수선해 줬으니 잘 입으시게.', { prev = false, next = true })
        end
    end
}
