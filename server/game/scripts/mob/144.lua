-- mob: 흑혈후
-- 흑혈후 사망
local quest = require('lib.quest')

function ON_MOB_KILL_144(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_144(me)
end
