local spell = require('lib.spell')

function ON_CAST_25(me, you, sp)
    local damage = math.random(48000, 50000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_25(me, sp)
end

function ON_UNBUFF_25(me, sp)
end
