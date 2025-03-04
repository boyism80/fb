-- 봉황의기원 캐스팅
function ON_CAST_4041(me, you, spell)
    local hp = 30000
    local mp = 10000
    local sound = 505
    local effect = 165
    spell_heal(me, you, spell, hp, mp, sound, effect)
end