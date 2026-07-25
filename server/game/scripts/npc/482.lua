-- npc: 황금서부백호
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.boss_challenge_gate(me, npc_obj)
    end
}
