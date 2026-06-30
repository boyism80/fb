-- spell: 뢰진주'첨
-- 뢰진주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3025(me, sp)
    local damage = 300
    local mp = 180
    local sound = 55
    local effect = 27
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
