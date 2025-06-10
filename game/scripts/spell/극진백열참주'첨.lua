-- 극진백열참주'첨 캐스팅
function ON_CAST_3040(me, spell)
    local damage = 3560
    local mp = 470
    local sound = 54
    local effect = 43
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end