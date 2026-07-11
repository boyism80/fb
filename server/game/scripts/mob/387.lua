-- mob: 달걀귀신
-- 달걀귀신 사망
local quest = require('lib.quest')

function ON_MOB_KILL_387(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_387(me)
-- end
