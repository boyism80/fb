-- 오중공격 캐스팅
function ON_CAST_1012(me, spell)
    local mp = 200
    local sound = 9
    local effect = 72
    local buff_time = 600

    if buff_cast(me, me, spell, mp, sound, effect) then
        me:buff(spell, buff_time)
    end
end

-- 오중공격 버프 효과
function ON_BUFF_1012(me, spell)
    me:unbuff('이중공격')
    me:unbuff('삼중공격')
    me:unbuff('사중공격')
    me:damage_rate(me:damage_rate() + 4000)
end

-- 오중공격 버프 해제 효과
function ON_UNBUFF_1012(me, spell)
    me:damage_rate(me:damage_rate() - 4000)

end