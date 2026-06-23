-- map: 폭염도
local quest = require('lib.quest')

function ON_MAP_MOVE_1698(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.goddess_dew_on_move(me)
end
