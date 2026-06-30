-- mob: 서현거미
-- 서현거미 사망
local quest = require('lib.quest')

function ON_MOB_DIE_1151(me, you)
    quest.king_on_mob_die(me, you)
end
