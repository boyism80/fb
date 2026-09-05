-- npc: 현무수련스승
local castle_npc = require('lib.castle_npc')

return {
    on_click = function(me, npc)
        castle_npc.on_trainer_click(me, npc)
    end
}
