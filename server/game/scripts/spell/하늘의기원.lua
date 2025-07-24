-- 하늘의기원 캐스팅
function ON_CAST_4029(me, spell)
    local hp = 200
    local mp = 120
    local sound = 3
    local effect = 5
    spell_heal(me, me, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end