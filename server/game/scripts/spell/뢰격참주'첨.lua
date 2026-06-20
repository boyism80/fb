-- 뢰격참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3033(me, sp)
    local damage = 720
    local mp = 330
    local sound = 57
    local effect = 29
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end