-- 화열참주'첨 캐스팅
function ON_CAST_3030(me, spell)
    local damage = 720
    local mp = 330
    local sound = 42
    local effect = 47
    spell_damage_near(me, spell, damage, mp, sound, effect)
end