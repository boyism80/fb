-- mob: 외눈황천구
-- 외눈황천구 사망
local quest = require('lib.quest')

function ON_MOB_DIE_141(me, you)
    quest.king_on_mob_die(me, you)
end
