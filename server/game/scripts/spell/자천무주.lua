-- 자천무주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3011(me, you, sp)
    local damage = 740
    local mp = 250
    local sound = 47
    local effect = 25
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end