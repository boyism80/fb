-- 화염주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3002(me, you, sp)
    local damage = 320
    local mp = 120
    local sound = 40
    local effect = 45
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end