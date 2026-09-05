-- npc: 청룡포목상
local castle_npc = require('lib.castle_npc')

return {
    on_click = function(me, npc)
        castle_npc.on_cloth_shop_click(me, npc)
    end
}
