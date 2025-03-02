-- 진뢰격참주'첨 캐스팅
function ON_CAST_3037(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 58
    local effect = 30
    spell_damage_near(me, spell, damage, mp, sound, effect)
end