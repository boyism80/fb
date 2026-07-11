-- mob: 흉노평민
-- 흉노평민 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1143(me, you)

end

-- 흉노평민 사망
function ON_MOB_KILL_1143(me, mobs)
    quest.king_on_mob_kill(me, mobs)
end

function ON_MOB_DIE_1143(me)
end
