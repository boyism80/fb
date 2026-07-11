-- mob: 독충
-- 독충 사망
local quest = require('lib.quest')

function ON_MOB_KILL_384(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_384(me)
end
