-- 자무주 캐스팅
function ON_CAST_3003(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 45
    local effect = 23
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end