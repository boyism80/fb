-- 진뢰격참주 캐스팅
function ON_CAST_3017(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 58
    local effect = 30
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end