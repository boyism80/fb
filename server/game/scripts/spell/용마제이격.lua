local spell = require('lib.spell')

function ON_CAST_22(me, you, sp)
    local damage = math.random(8000, 10000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_22(me, sp)
end

function ON_UNBUFF_22(me, sp)
end
