-- 극진자천무주'첨 캐스팅
function ON_CAST_3039(me, spell)
    local damage = 3560
    local mp = 470
    local sound = 49
    local effect = 26
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end