-- mob: 유령
-- 유령 사망
local quest = require('lib.quest')

function ON_MOB_KILL_352(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_352(me)
-- end
