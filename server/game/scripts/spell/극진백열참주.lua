-- 극진백열참주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3020(me, you, sp)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 54
    local effect = 43
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end