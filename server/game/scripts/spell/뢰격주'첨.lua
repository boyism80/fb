-- 뢰격주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3029(me, sp)
    local damage = 510
    local mp = 250
    local sound = 56
    local effect = 28
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end