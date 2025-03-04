-- 대력검신 캐스팅
function ON_CAST_1003(me, spell, message)
    local mp = 60
    local dam = 30
    spell_weapon_damage(me, mp, message, dam)
end