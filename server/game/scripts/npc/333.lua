-- npc: 현무갑옷상
local castle_npc = require('lib.castle_npc')

return {
    on_click = function(me, npc)
        castle_npc.on_armor_shop_click(me, npc)
    end
}
