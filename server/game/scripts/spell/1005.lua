-- spell: 신의축복
-- 신의축복 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local mp = 30
        local sound = 6
        local effect = 58
        local buff_time = 600
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    on_buff = function(me, sp)
        me:buff_hit(me:buff_hit() + 3)
    end,

    on_unbuff = function(me, sp)
        me:buff_hit(me:buff_hit() - 3)
    end,

    -- on_concast = function(me, sp)
    -- end
}
