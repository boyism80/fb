-- mob: 서현가재
-- 서현가재 사망
local quest = require('lib.quest')

function ON_MOB_KILL_341(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_341(me)
-- end
