-- map id: 1116
local quest = require('lib.quest')

function ON_MAP_MOVE_1116(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.manrihyang_seed_on_move(me)
end
