-- mob: 빗자루귀신
-- 빗자루귀신 사망
local quest = require('lib.quest')

function ON_MOB_KILL_385(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_385(me)
end
