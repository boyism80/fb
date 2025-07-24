-- 검신검귀 캐스팅
function ON_CAST_1002(me, spell, message)
    local mp = 60
    local dam = 20
    spell_weapon_damage(me, mp, message, dam)
end