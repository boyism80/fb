-- mob: 아기강시
-- 아기강시 사망
local quest = require('lib.quest')

function ON_MOB_KILL_148(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_148(me)
end
