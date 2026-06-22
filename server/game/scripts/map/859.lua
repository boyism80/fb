-- map id: 859
local quest = require('lib.quest')

function ON_MAP_MOVE_859(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.greatwall_repair_on_move(me)
end
