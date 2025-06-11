-- 자천무주 캐스팅
function ON_CAST_3011(me, you, spell)
    local damage = 740
    local mp = 250
    local sound = 47
    local effect = 25
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end