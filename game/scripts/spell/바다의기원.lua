-- 바다의기원 캐스팅
function ON_CAST_1024(me, you, spell)
    local hp = 100
    local mp = 50
    local sound = 3
    local effect = 5
    spell_heal(me, you, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end