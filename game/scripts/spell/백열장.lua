-- 백열장 캐스팅
function ON_CAST_6(me, you, spell)
    local damage = 280
    local mp = 80
    local sound = 8
    local effect = 86
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end