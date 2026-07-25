-- spell: 차폐
-- 차폐 캐스팅
local spell = require('lib.spell')

-- 차폐 버프 효과

-- 차폐 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 100
        local sound = 8
        local effect = 2
        local buff_time = 21
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    on_buff = function(me, sp)
        me:cover(true)
    end,

    on_unbuff = function(me, sp)
        me:cover(false)
    end,

    -- on_concast = function(me, sp)
    -- end
}
