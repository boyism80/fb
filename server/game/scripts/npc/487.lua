-- npc: 폭노폭노
local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '안녕하세요. 어떻게 오셨나요?',
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
                { '귀환', function(me, ch)
                    local map = name2map('국내성')
                    if map then
                        me:map(map, math.random(3, 8), math.random(5, 9))
                        return DIALOG_RESULT.NEXT
                    end
                    return me:dialog(ch, '이동할 수 없습니다.', { prev = false, next = true })
                end },
            },
        })
    end
}
