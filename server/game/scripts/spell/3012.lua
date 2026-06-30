-- spell: 백열참주
-- 백열참주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3012(me, you, sp)
    local damage = 740
    local mp = 250
    local sound = 52
    local effect = 42
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
