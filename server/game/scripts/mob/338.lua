-- mob: 가재장
-- 가재장 사망
local quest = require('lib.quest')

function ON_MOB_DIE_338(me, you)
    quest.king_on_mob_die(me, you)
end
