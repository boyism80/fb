-- mob: 유령
-- 유령 사망
local quest = require('lib.quest')

function ON_MOB_DIE_352(me, you)
    quest.king_on_mob_die(me, you)
end
