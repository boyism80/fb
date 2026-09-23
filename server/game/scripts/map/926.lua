-- map: 강서성
local quest = require('lib.quest')

return {
    -- on_map_init = function(me)
    -- end,

    on_map_move = function(me)
        quest.crown_prince_toys_on_move(me)
    end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end
}
