-- spell: 신검합일
-- 신검합일 캐스팅
local spell = require('lib.spell')

function ON_CAST_1001(me, sp, message)
    local mp = 60
    local dam = 10
    spell.weapon_damage(me, mp, message, dam)
end
