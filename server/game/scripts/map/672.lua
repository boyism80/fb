-- map: 대방성입구
local quest = require('lib.quest')

return {
    -- on_map_init = function(me)
    -- end,

    on_map_move = function(me)
        quest.dongchung_insam_on_move(me, '동충하초', '드디어 동충하초를 찾았다! 정말 구하기 힘들구나.')
    end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end
}
