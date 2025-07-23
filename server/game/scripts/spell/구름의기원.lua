-- 구름의기원 캐스팅
function ON_CAST_4032(me, you, spell)
    local hp = 500
    local mp = 120
    local sound = 66
    local effect = 66
    spell_heal(me, you, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end