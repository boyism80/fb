-- mob: 전갈장
-- 전갈장 사망
local quest = require('lib.quest')

function ON_MOB_KILL_340(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_340(me)
end
