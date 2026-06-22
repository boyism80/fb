-- npc: 혼례식도우미
local npc = require('lib.npc')

function NPC_221(me, npc_obj)
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