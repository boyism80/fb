-- mob: 전갈장
-- 전갈장 사망
local quest = require('lib.quest')

function ON_MOB_DIE_340(me, you)
    quest.king_on_mob_die(me, you)
end
