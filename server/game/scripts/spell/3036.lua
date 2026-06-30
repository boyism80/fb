-- spell: 진백열참주'첨
-- 진백열참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3036(me, sp)
    local damage = 1930
    local mp = 400
    local sound = 53
    local effect = 43
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
