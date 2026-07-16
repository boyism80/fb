-- spell: 용마제일격
local spell = require('lib.spell')

function ON_CAST_21(me, you, sp)
    local damage = math.random(2000, 2500)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

-- function ON_BUFF_21(me, sp)
-- end

-- function ON_UNBUFF_21(me, sp)
-- end
