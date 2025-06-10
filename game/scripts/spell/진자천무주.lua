-- 진자천무주 캐스팅
function ON_CAST_3015(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 48
    local effect = 26
    spell_damage(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end