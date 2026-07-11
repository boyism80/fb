-- mob: 전갈
-- 전갈 사망
local quest = require('lib.quest')

function ON_MOB_KILL_339(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_339(me)
end
