-- mob: 가재장
-- 가재장 사망
local quest = require('lib.quest')

function ON_MOB_KILL_338(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_338(me)
end
