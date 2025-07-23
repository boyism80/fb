-- 신검합일 캐스팅
function ON_CAST_1001(me, spell, message)
    local mp = 60
    local dam = 10
    spell_weapon_damage(me, mp, message, dam)
end