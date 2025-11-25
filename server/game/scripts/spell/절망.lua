-- 절망 캐스팅
function ON_CAST_3053(me, you, spell)
    local mp = 80
    local sound = 71
    local effect = 14
    local buff_time = 10
    if debuff_cast(me, you, spell, {mp = mp, sound = sound, effect = effect}) then
        you:buff(spell, buff_time, me)
    end
end

-- 절망 버프 효과
function ON_BUFF_3053(me, spell)
    me:add_cc(CROWD_CONTROL.SIGHT)
end

-- 절망 버프 해제 효과
function ON_UNBUFF_3053(me, spell)

    me:remove_cc(CROWD_CONTROL.SIGHT)
end

-- 절망 지속 효과
function ON_CONCAST_3053(me, caster, buff)

end

