-- npc: 길림미로보초
local door = require('lib.promotion_door')

return {
    on_click = function(me, npc)
        door.oak_skip(me, npc, 1774, 12, 22)
    end,
}
