-- spell: 뢰진주
-- 뢰진주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3005(me, you, sp)
    local damage = 320
    local mp = 120
    local sound = 55
    local effect = 27
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
