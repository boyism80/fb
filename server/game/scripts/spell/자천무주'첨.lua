-- 자천무주'첨 캐스팅
function ON_CAST_3031(me, spell)
    local damage = 720
    local mp = 330
    local sound = 47
    local effect = 25
    spell_damage_near(me, spell, {damage=damage, mp=mp, sound=sound, effect=effect})
end