-- 검신검귀 캐스팅
local spell = require('lib.spell')

function ON_CAST_1002(me, sp, message)
    local mp = 60
    local dam = 20
    spell.weapon_damage(me, mp, message, dam)
end