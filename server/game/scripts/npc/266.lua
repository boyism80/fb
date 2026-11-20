-- npc: 길림동굴보초
local door = require('lib.promotion_door')

return {
    on_click = function(me, npc)
        door.oak_skip(me, npc, 1758, 14, 7)
    end,
}
