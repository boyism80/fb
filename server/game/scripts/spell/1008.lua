-- spell: 백호령
-- 백호령 캐스팅
local spell = require('lib.spell')

-- 백호령 버프 효과

-- 백호령 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 30
        local sound = 12
        local effect = 2
        local buff_time = 600
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    on_buff = function(me, sp)
        me:buff_dam(me:buff_dam() + 3)
    end,

    on_unbuff = function(me, sp)
        me:buff_dam(me:buff_dam() - 3)
    end,

    -- on_concast = function(me, sp)
    -- end
}
