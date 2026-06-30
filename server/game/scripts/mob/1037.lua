-- mob: 선비검객
-- 선비검객 공격
local quest = require('lib.quest')

function ON_MOB_ATTACK_1037(me, you)
    return false
end

-- 선비검객 사망
function ON_MOB_DIE_1037(me, you)
    quest.king_on_mob_die(me, you)
end
