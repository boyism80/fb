-- map: 남경2
local quest = require('lib.quest')

function ON_MAP_MOVE_958(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.cheongsimcho_on_move(me)
end
