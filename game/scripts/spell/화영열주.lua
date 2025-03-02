-- 화영열주 캐스팅
function ON_CAST_3006(me, you, spell)
    local damage = 530
    local mp = 180
    local sound = 41
    local effect = 46
    spell_damage(me, you, spell, damage, mp, sound, effect)
end