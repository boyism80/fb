-- spell: 삼성백열장
-- 삼성백열장: deal magic damage to the selected target.
local spell = require('lib.spell')

function ON_CAST_105(me, you, sp)
    local damage = 796
    local mp = 420
    local sound = 8
    local effect = 86
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end
