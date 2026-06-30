-- mob: 흉노무사
-- 흉노무사 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1145(me, you)

end

-- 흉노무사 사망
function ON_MOB_DIE_1145(me, you)
    quest.king_on_mob_die(me, you)
end
