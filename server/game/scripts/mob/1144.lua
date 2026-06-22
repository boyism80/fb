-- mob: 흉노전사
-- 흉노전사 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1144(me, you)

end

-- 흉노전사 사망
function ON_MOB_DIE_1144(me, you)
    quest.king_on_mob_die(me, you)
end
