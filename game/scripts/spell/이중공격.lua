-- 이중공격 캐스팅
function ON_CAST_1009(me, spell)
    local mp = 60
    local sound = 9
    local effect = 70
    local buff_time = 600

    if buff_cast(me, me, spell, {mp=mp, sound=sound, effect=effect}) then
        me:buff(spell, buff_time)
    end
end

-- 이중공격 버프 효과
function ON_BUFF_1009(me, spell)
    me:unbuff('삼중공격')
    me:unbuff('사중공격')
    me:unbuff('오중공격')
    me:damage_rate(me:damage_rate() + 1000)
end

-- 이중공격 버프 해제 효과
function ON_UNBUFF_1009(me, spell)
    me:damage_rate(me:damage_rate() - 1000)

end