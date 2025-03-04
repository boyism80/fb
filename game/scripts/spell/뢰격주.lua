-- 뢰격주 캐스팅
function ON_CAST_3009(me, you, spell)
    local damage = 530
    local mp = 180
    local sound = 56
    local effect = 28
    spell_damage(me, you, spell, damage, mp, sound, effect)
end