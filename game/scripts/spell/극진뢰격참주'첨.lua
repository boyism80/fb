-- 극진뢰격참주'첨 캐스팅
function ON_CAST_3041(me, spell)
    local damage = 3560
    local mp = 470
    local sound = 59
    local effect = 30
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end