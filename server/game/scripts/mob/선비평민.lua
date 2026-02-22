-- 선비평민 공격
function ON_MOB_ATTACK_1036(me, you)
    return false
end

-- 선비평민 사망
function ON_MOB_DIE_1036(me, you)
    king_quest_on_mob_die(me, you)
end