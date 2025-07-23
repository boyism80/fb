-- 천공의기원 캐스팅
function ON_CAST_4030(me, you, spell)
    local hp = 200
    local mp = 100
    local sound = 34
    local effect = 66
    spell_heal(me, you, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end