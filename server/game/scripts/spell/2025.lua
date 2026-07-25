-- spell: 전혈
-- 전혈 캐스팅
local spell = require('lib.spell')

-- 전혈 버프 효과
-- 전혈 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 30
        local sound = 7
        local effect = 136
        local buff_time = 180
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time, me)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
