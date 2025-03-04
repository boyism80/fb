-- 태양의희원 캐스팅
function ON_CAST_4035(me, you, spell)
    local hp = 1000
    local mp = 960
    local sound = 76
    local effect = 66
    spell_heal_near(me, you, spell, hp, mp, sound, effect)
end