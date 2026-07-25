-- npc: 어버이날
local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '부모님께 카네이션을 선물하고 싶으시다구요?',
            menu = {
                { '꽃을 산다', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
            },
        })
    end
}
