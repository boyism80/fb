-- npc: 부윷일
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.yut_game(me, npc_obj)
    end
}
