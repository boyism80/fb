-- mob: 처녀귀신
-- 처녀귀신 사망
local quest = require('lib.quest')

function ON_MOB_DIE_386(me, you)
    quest.king_on_mob_die(me, you)
end
