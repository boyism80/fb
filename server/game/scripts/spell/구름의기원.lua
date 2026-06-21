-- 구름의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4032(me, you, sp)
    local hp = 500
    local mp = 120
    local sound = 66
    local effect = 66
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end