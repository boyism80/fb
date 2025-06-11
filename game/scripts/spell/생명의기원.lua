-- 생명의기원 캐스팅
function ON_CAST_4036(me, you, spell)
    local hp = 5000
    local mp = 300
    local sound = 98
    local effect = 65
    spell_heal(me, you, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end