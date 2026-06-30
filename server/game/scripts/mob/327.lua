-- mob: 구자호
-- 구자호 사망
local quest = require('lib.quest')

function ON_MOB_DIE_327(me, you)
    quest.king_on_mob_die(me, you)
end
