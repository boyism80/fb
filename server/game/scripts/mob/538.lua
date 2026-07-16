-- mob: 흡혈강시
-- 흡혈강시 사망
local quest = require('lib.quest')

function ON_MOB_KILL_538(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_538(me)
-- end
