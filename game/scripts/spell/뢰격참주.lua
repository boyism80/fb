-- 뢰격참주 캐스팅
function ON_CAST_3013(me, you, spell)
    local damage = 740
    local mp = 250
    local sound = 57
    local effect = 29
    spell_damage(me, you, spell, damage, mp, sound, effect)
end