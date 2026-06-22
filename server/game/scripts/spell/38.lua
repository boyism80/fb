-- spell: 용천제구격
local spell = require('lib.spell')

function ON_CAST_38(me, you, sp)
    local damage = math.random(250000, 280000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_38(me, sp)
end

function ON_UNBUFF_38(me, sp)
end
