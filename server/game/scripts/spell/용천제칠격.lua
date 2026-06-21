local spell = require('lib.spell')

function ON_CAST_36(me, you, sp)
    local damage = math.random(120000, 150000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_36(me, sp)
end

function ON_UNBUFF_36(me, sp)
end
