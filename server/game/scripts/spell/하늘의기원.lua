-- 하늘의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4029(me, sp)
    local hp = 200
    local mp = 120
    local sound = 3
    local effect = 5
    spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end