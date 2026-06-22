-- map id: 963
local quest = require('lib.quest')

function ON_MAP_MOVE_963(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.cheongsimcho_on_move(me)
end
