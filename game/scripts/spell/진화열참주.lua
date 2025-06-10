-- 진화열참주 캐스팅
function ON_CAST_3014(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 43
    local effect = 48
    spell_damage(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end