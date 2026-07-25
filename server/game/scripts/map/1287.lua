-- map: 도삭산808층
local quest = require('lib.quest')

return {
    -- on_map_init = function(me)
    -- end,

    on_map_move = function(me)
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        quest.mountain_treasure_map_on_move(me)
    end,

    -- on_map_enter = function(me)
    -- end,

    -- on_map_leave = function(me)
    -- end
}
