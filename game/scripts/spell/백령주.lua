-- 백령주 캐스팅
function ON_CAST_3008(me, you, spell)
    local damage = 530
    local mp = 180
    local sound = 51
    local effect = 41
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end