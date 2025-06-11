-- 자영무주 캐스팅
function ON_CAST_3007(me, you, spell)
    local damage = 530
    local mp = 180
    local sound = 46
    local effect = 24
    spell_damage(me, you, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end