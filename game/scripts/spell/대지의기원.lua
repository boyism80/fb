-- 대지의기원 캐스팅
function ON_CAST_4027(me, you, spell)
    local hp = 50
    local mp = 50
    local sound = 3
    local effect = 5
    spell_heal(me, you, spell, hp, mp, sound, effect)
end