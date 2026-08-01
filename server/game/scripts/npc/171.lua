-- npc: 고균도푸줏간
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
            },
        })
    end
}
