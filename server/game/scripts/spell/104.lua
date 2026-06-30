-- spell: 이성백열장
-- 이성백열장: deal magic damage to the selected target.
local spell = require('lib.spell')

function ON_CAST_104(me, you, sp)
    local damage = 592
    local mp = 300
    local sound = 8
    local effect = 86
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end
