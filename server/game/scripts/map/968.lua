-- map: 상해2
local quest = require('lib.quest')

function ON_MAP_MOVE_968(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.crown_prince_toys_on_move(me)
end
