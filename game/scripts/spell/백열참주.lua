-- 백열참주 캐스팅
function ON_CAST_3012(me, you, spell)
    local damage = 740
    local mp = 250
    local sound = 52
    local effect = 42
    spell_damage(me, you, spell, {damage,=damage mp=mp, sound=sound, effect=effect})
end