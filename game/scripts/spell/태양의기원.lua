-- 태양의기원 캐스팅
function ON_CAST_4034(me, you, spell)
    local hp = 1000
    local mp = 240
    local sound = 76
    local effect = 66
    spell_heal(me, you, spell, hp, mp, sound, effect)
end