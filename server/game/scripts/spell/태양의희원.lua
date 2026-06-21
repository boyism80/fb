-- 태양의희원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4035(me, you, sp)
    local hp = 1000
    local mp = 960
    local sound = 76
    local effect = 66
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end