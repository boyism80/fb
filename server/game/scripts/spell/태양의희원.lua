-- 태양의희원 캐스팅
function ON_CAST_4035(me, you, spell)
    local hp = 1000
    local mp = 960
    local sound = 76
    local effect = 66
    spell_heal(me, you, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end