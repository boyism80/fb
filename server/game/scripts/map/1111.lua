-- map id: 1111
local quest = require('lib.quest')

function ON_MAP_MOVE_1111(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.manrihyang_seed_on_move(me)
end
