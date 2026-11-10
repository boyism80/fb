-- map: 복건성
local quest = require('lib.quest')

return {
    -- on_map_init = function(me)
    -- end,

    on_map_move = function(me)
        quest.crown_prince_toys_on_move(me, '상아주사위', '상아로 만든 예쁜 주사위를 주웠다!')
    end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end
}
