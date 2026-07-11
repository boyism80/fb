-- mob: 처녀귀신
-- 처녀귀신 사망
local quest = require('lib.quest')

function ON_MOB_KILL_386(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_386(me)
-- end
