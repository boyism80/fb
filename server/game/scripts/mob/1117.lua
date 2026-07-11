-- mob: 거미랑
-- 거미랑 사망
local quest = require('lib.quest')

function ON_MOB_KILL_1117(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

-- function ON_MOB_DIE_1117(me)
-- end
