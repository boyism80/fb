-- 화영열주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3006(me, you, sp)
    local damage = 530
    local mp = 180
    local sound = 41
    local effect = 46
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end