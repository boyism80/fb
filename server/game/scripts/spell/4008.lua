-- spell: 심안투영
-- 심안투영 캐스팅
local spell = require('lib.spell')

-- 심안투영 버프 효과

-- 심안투영 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 30
        local sound = 78
        local effect = 11
        local buff_time = 600
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    on_buff = function(me, sp)
        me:detect(true)
    end,

    on_unbuff = function(me, sp)
        me:detect(false)
    end,

    -- on_concast = function(me, sp)
    -- end
}
