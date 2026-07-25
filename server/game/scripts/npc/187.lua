-- npc: 파바로
local enum = require('lib.enum')

return {
    on_click = function(me, npc)
        local GRILL_ITEMS = { '감자', '고구마', '늑대고기' }
        local GRILL_COST = 2

        local btn = me:dialog(npc, '구워먹기 좋은 음식은 내가 맛있게 금방 구워 주지. 대신 내게도 맛 좀 보게 해 주면 좋겠군. 음식 하나를 구워주면 수수료로 하나는 내가 먹겠어.', { prev = false, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end

        ::NPC_187_0002::
        local list = me:list(npc, '내가 전에 구워봤던 건 이 정도야. 굽고 싶은 음식이 있으면 골라보게. 나중에 또 구워봐서 성공하는 음식이 있으면 목록에 추가하도록 하지.', { '감자', '고구마', '늑대고기', '나중에 다시 올께요.' })
        if list == nil then
            return
        end
        if list == 4 then
            return
        end
        if list < 1 or list > 3 then
            return
        end

        local item_name = GRILL_ITEMS[list]
        ::NPC_187_0003::
        btn = me:dialog(npc, item_name .. '! 그거 좋지. 감자는 구우면 씹기도 좋고, 고소한 맛이 더 살아난다네.', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_187_0002
        end
        btn = me:dialog(npc, '자 그럼 어디 구워 볼까!', { prev = true, next = true })
        if btn == DIALOG_RESULT.QUIT then
            return
        end
        if btn == DIALOG_RESULT.PREV then
            goto NPC_187_0003
        end

        local code = me:exchange(
            { ['item'] = { [item_name] = GRILL_COST } },
            { ['item'] = { ['구운' .. item_name] = 1 } }
        )
        if code == enum.EXCHANGE_RESULT.LACK_COST then
            me:dialog(npc, name_with(item_name, '이', '가') .. ' 부족한 것 같은데?', { prev = false, next = true })
            return
        end
        if code == enum.EXCHANGE_RESULT.LACK_CAPACITY then
            me:dialog(npc, '소지품이 가득 차서 구운 음식을 줄 수 없네.', { prev = false, next = true })
            return
        end
        me:dialog(npc, '자, 다 됐네. 김이 모락모락 나는 것이 아주 잘 구워진 것 같군! 약속대로 내가 하나 먹고, 하나는 자네가 먹게나.', { prev = false, next = true })
    end
}
