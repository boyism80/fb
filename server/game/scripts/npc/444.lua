-- npc: 성황할머니
local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.revive_oath(me, npc_obj)
    end
}
