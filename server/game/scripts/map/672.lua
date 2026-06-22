-- map id: 672
local quest = require('lib.quest')

function ON_MAP_MOVE_672(me)
    if not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end

    quest.dongchung_insam_on_move(me)
end
