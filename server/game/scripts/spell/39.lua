-- spell: 용의제일주
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local mp = 40
        local sound = 40
        local effect = 53
        local buff_time = 6
        if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
            you:buff(sp, buff_time, me)
        end
    end,

    on_buff = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 31)
    end,

    on_unbuff = function(me, sp)
        me:buff_phydef(me:buff_phydef() - 31)
    end,

    -- on_concast = function(me, sp)
    -- end
}
