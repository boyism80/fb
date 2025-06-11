-- 뢰진주 캐스팅
function ON_CAST_3005(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 55
    local effect = 27
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end