-- 극진자천무주 캐스팅
function ON_CAST_3019(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 49
    local effect = 26
    spell_damage(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end