-- 진백열참주'첨 캐스팅
function ON_CAST_3036(me, spell)
    local damage = 1930
    local mp = 400
    local sound = 53
    local effect = 43
    spell_damage_near(me, spell, damage, mp, sound, effect)
end