-- mob: 사마귀
-- 사마귀 사망
local quest = require('lib.quest')

function ON_MOB_KILL_334(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_334(me)
end
