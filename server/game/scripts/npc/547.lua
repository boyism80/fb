-- npc: 암상인
local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '.....',
            menu = {
                { '물건 팔기', function(me, ch)
                    return npc.show_buy_menu(me, ch)
                end },
            },
        })
    end
}
