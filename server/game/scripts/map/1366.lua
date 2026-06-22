-- map id: 1366
local quest = require('lib.quest')

function ON_MAP_MOVE_1366(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.mountain_treasure_map_on_move(me)
end
