-- 후면공격 캐스팅
function ON_CAST_1006(me, spell)
    local mp = 90
    local sound = 7
    local effect = 2
    local buff_time = 625
    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

-- 후면공격 버프 효과
function ON_BUFF_1006(me, spell)

end

-- 후면공격 버프 해제 효과
function ON_UNBUFF_1006(me, spell)

end