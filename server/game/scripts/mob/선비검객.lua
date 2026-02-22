-- 선비검객 공격
function ON_MOB_ATTACK_1037(me, you)
    return false
end

-- 선비검객 사망
function ON_MOB_DIE_1037(me, you)
    king_quest_on_mob_die(me, you)
end