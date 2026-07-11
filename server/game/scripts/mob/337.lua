-- mob: 가재
-- 가재 사망
local quest = require('lib.quest')

function ON_MOB_KILL_337(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_337(me)
end
