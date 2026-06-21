local spell = require('lib.spell')

function ON_CAST_32(me, you, sp)
    local damage = math.random(18000, 20000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_32(me, sp)
end

function ON_UNBUFF_32(me, sp)
end
