-- 진자천무주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3035(me, sp)
    local damage = 1930
    local mp = 400
    local sound = 48
    local effect = 26
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end