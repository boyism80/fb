-- spell: 극진백열참주'첨
-- 극진백열참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3040(me, sp)
    local damage = 3560
    local mp = 470
    local sound = 54
    local effect = 43
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
