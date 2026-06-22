-- spell: 백령주
-- 백령주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3008(me, you, sp)
    local damage = 530
    local mp = 180
    local sound = 51
    local effect = 41
    spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
