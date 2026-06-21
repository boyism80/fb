-- 극진자천무주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3039(me, sp)
    local damage = 3560
    local mp = 470
    local sound = 49
    local effect = 26
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end