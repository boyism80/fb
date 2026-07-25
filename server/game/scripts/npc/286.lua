-- npc: 부윷이
local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.yut_game(me, npc_obj)
    end
}
