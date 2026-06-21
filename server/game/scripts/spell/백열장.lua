-- 백열장 캐스팅
local spell = require('lib.spell')

function ON_CAST_6(me, you, sp)
    local damage = 280
    local mp = 80
    local sound = 8
    local effect = 86
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end