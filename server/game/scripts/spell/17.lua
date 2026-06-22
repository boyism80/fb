-- spell: 시약보호
local spell = require('lib.spell')

function ON_CAST_17(me, you, sp)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if spell.buff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
        you:buff(sp, buff_time, me)
    end
end

function ON_BUFF_17(me, sp)
    me:damage_derate(me:damage_derate() + 1000)
end

function ON_UNBUFF_17(me, sp)
    me:damage_derate(me:damage_derate() - 1000)
end
