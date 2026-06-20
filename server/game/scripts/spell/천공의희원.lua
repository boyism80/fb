-- 천공의희원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4031(me, you, sp)
    local hp = 200
    local mp = 200
    local sound = 34
    local effect = 66
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end