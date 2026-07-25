-- spell: 분신
-- 분신 캐스팅
local spell = require('lib.spell')

-- 분신 버프 효과
-- 분신 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 0
        local sound = 39
        local effect = 11
        local buff_time = 625
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
