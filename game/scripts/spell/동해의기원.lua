-- 동해의기원 캐스팅
function ON_CAST_4028(me, you, spell)
    local hp = 100
    local mp = 30
    local sound = 5
    local effect = 5
    spell_heal(me, you, spell, hp, mp, sound, effect)
end