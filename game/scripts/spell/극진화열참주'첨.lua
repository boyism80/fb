-- 극진화열참주'첨 캐스팅
function ON_CAST_3038(me, spell)
    local damage = 3560
    local mp = 470
    local sound = 44
    local effect = 48
    spell_damage_near(me, spell, damage, mp, sound, effect)
end