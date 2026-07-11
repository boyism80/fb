-- mob: 칼든해골
-- 칼든해골 사망
local quest = require('lib.quest')

function ON_MOB_KILL_347(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_347(me)
end
