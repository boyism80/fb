-- spell: 용의제사주
-- 용의제사주: debuff (curse), AC 45, 6s, effect 53, sound 40.
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local mp = 40
        local sound = 40
        local effect = 53
        local buff_time = 6
        if spell.debuff_cast(me, you, sp, { mp = mp, sound = sound, effect = effect }) then
            you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 45)
    end,

    ON_UNBUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() - 45)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
