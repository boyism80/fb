-- spell: 누리의기원
-- 누리의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4025(me, sp)
    local hp = 50
    local mp = 30
    local sound = 3
    local effect = 5
    spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end
