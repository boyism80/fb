-- 대력검신 캐스팅
local spell = require('lib.spell')

function ON_CAST_1003(me, sp, message)
    local mp = 60
    local dam = 30
    spell.weapon_damage(me, mp, message, dam)
end