-- npc: 동쪽태극문보초
local door = require('lib.promotion_door')

return {
    on_click = function(me, npc)
        door.oak_skip(me, npc, 1772, 13, 13)
    end,
}
