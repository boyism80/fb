-- 극진백열참주 캐스팅
function ON_CAST_3020(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 54
    local effect = 43
    spell_damage(me, you, spell, damage, mp, sound, effect)
end