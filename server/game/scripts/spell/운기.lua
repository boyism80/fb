-- 운기 캐스팅
function ON_CAST_2024(me, spell)
    local mp = 30
    local sound = 2
    local effect = nil
    local buff_time = 11
    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time, me)
    end
end

-- 운기 버프 효과
function ON_BUFF_2024(me, spell)

end

-- 운기 버프 해제 효과
function ON_UNBUFF_2024(me, spell)

end

-- 운기 지속 효과
function ON_CONCAST_2024(me, caster, buff)
    me:effect(133)
    me:sound(2)
    me:mp_up(me:maxmp() // 10)
end