-- spell: 차폐
-- 차폐 캐스팅
local spell = require('lib.spell')

-- 차폐 버프 효과

-- 차폐 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 100
        local sound = 8
        local effect = 2
        local buff_time = 21
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    ON_BUFF = function(me, sp)
        me:cover(true)
    end,

    ON_UNBUFF = function(me, sp)
        me:cover(false)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
