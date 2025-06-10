-- 누리의기원 캐스팅
function ON_CAST_4025(me, spell)
    local hp = 50
    local mp = 30
    local sound = 3
    local effect = 5
    spell_heal(me, me, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end