-- spell: 극진자천무주
-- 극진자천무주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3019(me, you, sp)
    local damage = 3580 -- + me:int()*5
    local mp = 360
    local sound = 49
    local effect = 26
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
