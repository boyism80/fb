-- mob: 거미
-- 거미 사망
local quest = require('lib.quest')

function ON_MOB_KILL_333(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_333(me)
-- end
