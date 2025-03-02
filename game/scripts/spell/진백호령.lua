-- 진백호령 캐스팅
function ON_CAST_1013(me, spell)
    local mp = 30
    local sound = 89
    local effect = 57
    local buff_time = 60

    if not buff_cast(me, me, spell, mp, sound, effect) then
        return
    end
    me:buff(spell, buff_time)
end

-- 진백호령 버프 효과
function ON_BUFF_1013(me, spell)
    me:skill_damage_rate(me:skill_damage_rate() + 500)
end

-- 진백호령 버프 해제 효과
function ON_UNBUFF_1013(me, spell)
    me:skill_damage_rate(me:skill_damage_rate() - 500)

end