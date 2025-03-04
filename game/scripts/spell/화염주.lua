-- 화염주 캐스팅
function ON_CAST_3002(me, you, spell)
    local damage = 320
    local mp = 120
    local sound = 40
    local effect = 45
    spell_damage(me, you, spell, damage, mp, sound, effect)
end