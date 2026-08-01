-- npc: 백두촌대장장이
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
                { '물건 고치기', function(me, ch)
                    return npc.show_repair_menu(me, ch)
                end },
                { '모두 고치기', function(me, ch)
                    return npc.show_repair_all_menu(me, ch)
                end },
                { '무기 이름 명명', function(me, ch)
                    return npc.show_rename_weapon_menu(me, ch)
                end },
            },
        })
    end
}
