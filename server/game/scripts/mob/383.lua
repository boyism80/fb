-- mob: 몽달귀신
-- 몽달귀신 사망
local quest = require('lib.quest')

function ON_MOB_KILL_383(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_383(me)
-- end
