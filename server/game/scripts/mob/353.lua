-- mob: 중급유령
-- 중급유령 사망
local quest = require('lib.quest')

function ON_MOB_KILL_353(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_353(me)
-- end
