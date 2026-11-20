-- npc: 부구륜
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.promotion_master(me, npc_obj, CLASS.ROGUE)
    end
}
