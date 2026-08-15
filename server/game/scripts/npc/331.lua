-- npc: 현무대장장이
local castle_npc = require('lib.castle_npc')

return {
    on_click = function(me, npc)
        castle_npc.on_blacksmith_click(me, npc)
    end
}
