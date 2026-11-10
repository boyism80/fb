-- map: 상해1
local quest = require('lib.quest')

return {
    -- on_map_init = function(me)
    -- end,

    on_map_move = function(me)
        quest.crown_prince_toys_on_move(me, '오색폭죽', '다섯가지 색 불꽃이 나는 신기한 폭죽을 주웠다!')
    end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end
}
