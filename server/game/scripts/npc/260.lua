-- npc: 도삭산900층주모
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '안녕하세요. 어떻게 오셨나요?',
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
                { '금전 맡기기', function(me, ch)
                    return npc.show_hold_money_menu(me, ch)
                end },
                { '금전 되찾기', function(me, ch)
                    return npc.show_return_money_menu(me, ch)
                end },
                { '물건 맡기기', function(me, ch)
                    return npc.show_hold_item_menu(me, ch)
                end },
                { '물건 되찾기', function(me, ch)
                    return npc.show_return_item_menu(me, ch)
                end },
            },
        })
    end
}
