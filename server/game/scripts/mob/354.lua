-- mob: 고급유령
-- 고급유령 사망
local quest = require('lib.quest')

function ON_MOB_DIE_354(me, you)
    quest.king_on_mob_die(me, you)
end
