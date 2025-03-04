-- 극진뢰격참주 캐스팅
function ON_CAST_3021(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 59
    local effect = 30
    spell_damage(me, you, spell, damage, mp, sound, effect)
end