-- 진자천무주'첨 캐스팅
function ON_CAST_3035(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 48
    local effect = 26
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end