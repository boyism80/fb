-- spell: 시약보호
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local mp = 30
        local sound = 8
        local effect = 110
        local buff_time = 180
        if spell.buff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
            you:buff(sp, buff_time, me)
        end
    end,

    on_buff = function(me, sp)
        me:damage_derate(me:damage_derate() + 1000)
    end,

    on_unbuff = function(me, sp)
        me:damage_derate(me:damage_derate() - 1000)
    end,

    -- on_concast = function(me, sp)
    -- end
}
