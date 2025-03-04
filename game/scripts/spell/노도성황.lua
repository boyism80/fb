-- 노도성황 캐스팅
function ON_CAST_3049(me, you, spell)
    local mp = 30
    local sound = 106
    local effect = 96
    local buff_time = 30
    if debuff_cast(me, you, spell, mp, sound, effect) then
        you:buff(spell, buff_time, me)
    end
end

-- 노도성황 버프 효과
function ON_BUFF_3049(me, spell)

end

-- 노도성황 버프 해제 효과
function ON_UNBUFF_3049(me, spell)

end

-- 노도성황 지속 효과
function ON_CONCAST_3049(me, caster, buff)
    local damage = (me:maxhp() * 5) // 100
    me:damage(damage, caster)
    me:effect(96)
end