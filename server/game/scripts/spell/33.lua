-- spell: 용천제사격
local spell = require('lib.spell')

function ON_CAST_33(me, you, sp)
    local damage = math.random(40000, 45000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

-- function ON_BUFF_33(me, sp)
-- end

-- function ON_UNBUFF_33(me, sp)
-- end
