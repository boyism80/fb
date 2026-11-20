-- npc: 서쪽태극문보초
local door = require('lib.promotion_door')

return {
    on_click = function(me, npc)
        door.oak_skip(me, npc, 1773, 13, 13)
    end,
}
