-- spell: 진화열참주'첨
-- 진화열참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3034(me, sp)
    local damage = 1930
    local mp = 400
    local sound = 43
    local effect = 48
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
