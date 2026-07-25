local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.shop(me, npc_obj, {
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
                { '물건 맡기기', function(me, ch)
                    return npc.show_hold_item_menu(me, ch)
                end },
                { '금전 찾기', function(me, ch)
                    return npc.show_return_money_menu(me, ch)
                end },
                { '물건 찾기', function(me, ch)
                    return npc.show_return_item_menu(me, ch)
                end },
            },
        })
    end
}
