-- spell: 용마제육격
local spell = require('lib.spell')

function ON_CAST_26(me, you, sp)
    local damage = math.random(78000, 80000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

-- function ON_BUFF_26(me, sp)
-- end

-- function ON_UNBUFF_26(me, sp)
-- end
