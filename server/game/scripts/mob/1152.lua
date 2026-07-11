-- mob: 초급유령
-- 초급유령 사망
local quest = require('lib.quest')

function ON_MOB_KILL_1152(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_1152(me)
end
