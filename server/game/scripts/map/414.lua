-- map: 사마귀굴3
local dungeon_chat = require('lib.dungeon_enter_chat')

return {
    on_map_enter = dungeon_chat.on_enter,
}
