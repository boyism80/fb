-- mob: 사마귀랑
-- 사마귀랑 사망
local quest = require('lib.quest')

function ON_MOB_KILL_1118(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_1118(me)
-- end
