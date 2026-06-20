-- 백열주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3004(me, you, sp)
    local damage = 320
    local mp = 120
    local sound = 50
    local effect = 40
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end