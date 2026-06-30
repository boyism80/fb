-- spell: 자무주
-- 자무주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3003(me, you, sp)
    local damage = 320
    local mp = 120
    local sound = 45
    local effect = 23
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
