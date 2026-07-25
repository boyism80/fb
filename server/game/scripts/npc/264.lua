-- npc: 프리셋테스트4
local npc = require('lib.npc')

return {
    ON_CLICK = function(me, npc_obj)
        npc.nakrang_intro(me, npc_obj)
    end
}
