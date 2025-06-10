-- 극진화열참주 캐스팅
function ON_CAST_3018(me, you, spell)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 44
    local effect = 48
    spell_damage(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end