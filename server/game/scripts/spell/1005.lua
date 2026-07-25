-- spell: 신의축복
-- 신의축복 캐스팅
local spell = require('lib.spell')

-- 신의축복 버프 효과

-- 신의축복 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 30
        local sound = 6
        local effect = 58
        local buff_time = 600
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    ON_BUFF = function(me, sp)
        me:hit(me:hit()+3)
    end,

    ON_UNBUFF = function(me, sp)
        me:hit(me:hit()-3)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
