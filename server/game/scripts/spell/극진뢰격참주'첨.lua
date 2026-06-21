-- 극진뢰격참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3041(me, sp)
    local damage = 3560
    local mp = 470
    local sound = 59
    local effect = 30
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end