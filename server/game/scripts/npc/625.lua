-- npc: 문파성선생
local castle_npc = require('lib.castle_npc')

return {
    on_click = function(me, npc)
        castle_npc.on_sensei_click(me, npc)
    end
}
