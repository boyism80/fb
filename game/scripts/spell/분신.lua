-- 분신 캐스팅
function ON_CAST_2013(me, spell)
    local mp = 0
    local sound = 39
    local effect = 11
    local buff_time = 625
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

-- 분신 버프 효과
function ON_BUFF_2013(me, spell)

end

-- 분신 버프 해제 효과
function ON_UNBUFF_2013(me, spell)

end