-- npc: 도깨비상인
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.shop(me, npc_obj, {
            greeting = '난 비밀스러운 방망이를 파는 도깨비라네...',
            menu = {
                { '물건 사기', function(me, ch)
                    return npc.show_sell_menu(me, ch)
                end },
            },
        })
    end
}
