-- 뢰격주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3009(me, you, sp)
    local damage = 530
    local mp = 180
    local sound = 56
    local effect = 28
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end