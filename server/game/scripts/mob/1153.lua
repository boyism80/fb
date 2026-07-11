-- mob: 날쌘해골
-- 날쌘해골 사망
local quest = require('lib.quest')

function ON_MOB_KILL_1153(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_1153(me)
end
