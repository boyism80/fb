-- 바다의기원 캐스팅
function ON_CAST_4026(me, spell)
    local hp = 100
    local mp = 50
    local sound = 3
    local effect = 5
    spell_heal(me, me, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end