-- map: 사마귀굴2
local dungeon_chat = require('lib.dungeon_enter_chat')

return {
    on_map_enter = dungeon_chat.on_enter,
}
