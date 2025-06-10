-- 자영무주'첨 캐스팅
function ON_CAST_3027(me, spell)
    local damage = 510
    local mp = 250
    local sound = 46
    local effect = 24
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end