-- 백열주 캐스팅
function ON_CAST_3004(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 50
    local effect = 40
    spell_damage(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end