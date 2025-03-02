-- 백열주'첨 캐스팅
function ON_CAST_3024(me, spell)
    local damage = 300
    local mp = 180
    local sound = 50
    local effect = 40
    spell_damage_near(me, spell, damage, mp, sound, effect)
end