-- spell: 일성백열장
-- 일성백열장: deal magic damage to the selected target.
local spell = require('lib.spell')

function ON_CAST_103(me, you, sp)
    local damage = 424
    local mp = 180
    local sound = 8
    local effect = 86
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end
