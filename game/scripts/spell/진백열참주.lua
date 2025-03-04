-- 진백열참주 캐스팅
function ON_CAST_3016(me, you, spell)
    local damage = 1950
    local mp = 330
    local sound = 53
    local effect = 43
    spell_damage(me, you, spell, damage, mp, sound, effect)
end