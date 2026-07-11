-- mob: 친자호
-- 친자호 사망
local quest = require('lib.quest')

function ON_MOB_KILL_326(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_326(me)
end
