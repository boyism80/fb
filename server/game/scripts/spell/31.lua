-- spell: 용천제이격
local spell = require('lib.spell')

function ON_CAST_31(me, you, sp)
    local damage = math.random(8000, 10000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

-- function ON_BUFF_31(me, sp)
-- end

-- function ON_UNBUFF_31(me, sp)
-- end
