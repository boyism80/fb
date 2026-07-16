-- spell: 용천제팔격
local spell = require('lib.spell')

function ON_CAST_37(me, you, sp)
    local damage = math.random(170000, 200000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

-- function ON_BUFF_37(me, sp)
-- end

-- function ON_UNBUFF_37(me, sp)
-- end
