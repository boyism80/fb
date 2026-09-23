-- map: 귀기서린집
local quest = require('lib.quest')

return {
    -- on_map_init = function(me)
    -- end,

    on_map_move = function(me)
        quest.ghost_talisman_on_move(me)
    end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end
}
