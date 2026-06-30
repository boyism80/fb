-- mob: 녹혈후
-- 녹혈후 사망
local quest = require('lib.quest')

function ON_MOB_DIE_145(me, you)
    quest.king_on_mob_die(me, you)
end
