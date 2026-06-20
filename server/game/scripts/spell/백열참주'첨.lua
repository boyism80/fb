-- 백열참주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3032(me, sp)
    local damage = 720
    local mp = 330
    local sound = 52
    local effect = 42
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end