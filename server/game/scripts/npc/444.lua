-- npc: 성황할머니
local npc = require('lib.npc')

return {
    on_click = function(me, npc_obj)
        npc.revive_oath(me, npc_obj)
    end
}
