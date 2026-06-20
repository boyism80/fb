-- 대지의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4027(me, you, sp)
    local hp = 50
    local mp = 50
    local sound = 3
    local effect = 5
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end