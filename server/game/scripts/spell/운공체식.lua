-- 운공체식 캐스팅
function ON_CAST_1023(me, spell)
    local hp = 10000
    local mp = 10000
    local sound = 61
    local effect = 166
    spell_heal(me, me, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end