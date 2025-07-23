-- 구름의희원 캐스팅
function ON_CAST_4033(me, you, spell)
    local hp = 500
    local mp = 480
    local sound = 66
    local effect = 66
    spell_heal(me, you, spell, {hp=hp, mp=mp, sound=sound, effect=effect})
end