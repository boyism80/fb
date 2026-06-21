-- 뢰격참주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3013(me, you, sp)
    local damage = 740
    local mp = 250
    local sound = 57
    local effect = 29
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end