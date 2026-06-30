-- spell: 용마제칠격
local spell = require('lib.spell')

function ON_CAST_27(me, you, sp)
    local damage = math.random(120000, 150000)
    local mp = 100
    local sound = 8
    local effect = 51
    spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
end

function ON_BUFF_27(me, sp)
end

function ON_UNBUFF_27(me, sp)
end
