-- map: 귀기서린집
local quest = require('lib.quest')

function ON_MAP_MOVE_2535(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.ghost_talisman_on_move(me)
end
