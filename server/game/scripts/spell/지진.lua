-- 지진 캐스팅
local spell = require('lib.spell')

function ON_CAST_4042(me, you, sp)
    local damage = 1500
    local mp = 580
    local sound = 82
    local effect = 17
    spell.damage_near_target(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end