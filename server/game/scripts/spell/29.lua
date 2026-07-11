-- spell: 용마제구격
local spell = require('lib.spell')

function ON_CAST_29(me, you, sp)
    local damage = math.random(250000, 280000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

-- function ON_BUFF_29(me, sp)
-- end

-- function ON_UNBUFF_29(me, sp)
-- end
