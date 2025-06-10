-- 진화열참주'첨 캐스팅
function ON_CAST_3034(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 43
    local effect = 48
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end