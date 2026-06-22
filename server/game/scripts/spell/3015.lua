-- spell: 진자천무주
-- 진자천무주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3015(me, you, sp)
    local damage = 1950
    local mp = 330
    local sound = 48
    local effect = 26
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
