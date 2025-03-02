-- 화영열주'첨 캐스팅
function ON_CAST_3026(me, spell)
    local damage = 510
    local mp = 250
    local sound = 41
    local effect = 46
    spell_damage_near(me, spell, damage, mp, sound, effect)
end