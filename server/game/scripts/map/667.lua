-- map: 용왕용천의침소
local dragon_weapon = require('lib.dragon_weapon')

return {
    on_map_move = function(me)
        dragon_weapon.on_lair_move(me)
    end,
}
