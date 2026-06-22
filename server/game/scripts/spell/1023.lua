-- spell: 운공체식
-- 운공체식 캐스팅
local spell = require('lib.spell')

function ON_CAST_1023(me, sp)
    local hp = 10000
    local mp = 10000
    local sound = 61
    local effect = 166
    spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end
