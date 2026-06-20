-- 화열참주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3010(me, you, sp)
    local damage = 740
    local mp = 250
    local sound = 42
    local effect = 47
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end