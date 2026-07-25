-- map: 도삭산306층
local quest = require('lib.quest')

return {
    -- ON_MAP_INIT = function(me)
    -- end,

    ON_MAP_MOVE = function(me)
        if not me:is(OBJECT_TYPE.CHARACTER) then
            return
        end

        quest.manrihyang_seed_on_move(me)
    end,

    -- ON_MAP_ENTER = function(me)
    -- end,

    -- ON_MAP_LEAVE = function(me)
    -- end
}
