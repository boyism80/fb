-- 사성백열장: deal magic damage to the selected target.
local spell = require('lib.spell')

function ON_CAST_106(me, you, sp)
    local damage = 1064
    local mp = 550
    local sound = 8
    local effect = 86
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end
