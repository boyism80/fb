-- mob: 외눈황천구
-- 외눈황천구 사망
local quest = require('lib.quest')

function ON_MOB_KILL_141(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_141(me)
end
