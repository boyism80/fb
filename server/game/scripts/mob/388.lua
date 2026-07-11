-- mob: 불귀신
-- 불귀신 사망
local quest = require('lib.quest')

function ON_MOB_KILL_388(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_388(me)
-- end
