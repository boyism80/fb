-- spell: 투명
-- 투명 캐스팅
local spell = require('lib.spell')

-- 투명 버프 효과
-- 투명 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 0
        local sound = 28
        local effect = 20
        local buff_time = 180
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:state(STATE.CLOACK)
            me:buff(sp, buff_time)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    on_unbuff = function(me, sp)
        me:state(STATE.NORMAL)
    end,

    -- on_concast = function(me, sp)
    -- end
}
