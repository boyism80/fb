-- map: 국경지대
local quest = require('lib.quest')

function ON_MAP_MOVE_671(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.dongchung_insam_on_move(me)
end
