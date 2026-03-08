-- 중독 캐스팅
function ON_CAST_3059(me, you, spell)
    local mp = 30
    local sound = 68
    local effect = 14
    local buff_time = 30
    if debuff_cast(me, you, spell, {mp = mp, sound = sound, effect = effect}) then
        you:buff(spell, buff_time, me)
    end
end

-- 중독 버프 효과
function ON_BUFF_3059(me, spell)

end

-- 중독 버프 해제 효과
function ON_UNBUFF_3059(me, spell)

end

-- 중독 지속 효과
function ON_CONCAST_3059(me, caster, buff)
    local damage = math.min((me:maxhp() * 5) // 100, 14994)
    if damage > me:hp() - 100 then
        damage = math.max(0, me:hp() - 100)
    end
    me:damage(damage, caster, { critical = false, rate = caster:skill_damage_rate() / 1000.0, physical = false })
end