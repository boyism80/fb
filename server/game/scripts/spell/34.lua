-- spell: 용천제오격
local spell = require('lib.spell')

function ON_CAST_34(me, you, sp)
    local damage = math.random(48000, 50000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_34(me, sp)
end

function ON_UNBUFF_34(me, sp)
end
