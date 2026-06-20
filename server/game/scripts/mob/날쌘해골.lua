-- 날쌘해골 사망
local quest = require('lib.quest')

function ON_MOB_DIE_1153(me, you)
    quest.king_on_mob_die(me, you)
end