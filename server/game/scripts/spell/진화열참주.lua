-- 진화열참주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3014(me, you, sp)
    local damage = 1950
    local mp = 330
    local sound = 43
    local effect = 48
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end