-- mob: 흉노평민
-- 흉노평민 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1143(me, you)

end

-- 흉노평민 사망
function ON_MOB_DIE_1143(me, you)
    quest.king_on_mob_die(me, you)
end
