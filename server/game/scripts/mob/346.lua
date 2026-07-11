-- mob: 해골
-- 해골 사망
local quest = require('lib.quest')

function ON_MOB_KILL_346(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_346(me)
end
