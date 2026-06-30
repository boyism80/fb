-- spell: 화열참주'첨
-- 화열참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3030(me, sp)
    local damage = 720
    local mp = 330
    local sound = 42
    local effect = 47
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
