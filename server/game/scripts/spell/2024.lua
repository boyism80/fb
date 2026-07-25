-- spell: 운기
-- 운기 캐스팅
local spell = require('lib.spell')

-- 운기 버프 효과
-- 운기 버프 해제 효과
-- 운기 지속 효과

return {
    ON_CAST = function(me, sp)
        local mp = 30
        local sound = 2
        local effect = nil
        local buff_time = 11
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time, me)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    ON_CONCAST = function(me, caster, buff)
        me:effect(133)
        me:sound(2)
        me:mp_up(me:maxmp() // 10)
    end
}
