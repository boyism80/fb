-- 뢰진주'첨 캐스팅
function ON_CAST_3025(me, spell)
    local damage = 300
    local mp = 180
    local sound = 55
    local effect = 27
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end