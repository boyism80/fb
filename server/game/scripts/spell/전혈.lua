-- 전혈 캐스팅
function ON_CAST_2025(me, spell)
    local mp = 30
    local sound = 7
    local effect = 136
    local buff_time = 180
    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time, me)
    end
end

-- 전혈 버프 효과
function ON_BUFF_2025(me, spell)

end

-- 전혈 버프 해제 효과
function ON_UNBUFF_2025(me, spell)

end