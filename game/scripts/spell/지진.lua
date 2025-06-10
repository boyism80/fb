-- 지진 캐스팅
function ON_CAST_4042(me, you, spell)
    local damage = 1500
    local mp = 580
    local sound = 82
    local effect = 17
    spell_damage_near_target(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end