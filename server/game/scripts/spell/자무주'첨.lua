-- 자무주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3023(me, sp)
    local damage = 300
    local mp = 180
    local sound = 45
    local effect = 23
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end