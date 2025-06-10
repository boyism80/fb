-- 뢰격주'첨 캐스팅
function ON_CAST_3029(me, spell)
    local damage = 510
    local mp = 250
    local sound = 56
    local effect = 28
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end