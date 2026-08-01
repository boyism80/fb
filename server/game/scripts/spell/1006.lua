-- spell: 후면공격
-- 후면공격 캐스팅
local spell = require('lib.spell')

-- 후면공격 버프 효과
-- 후면공격 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 90
        local sound = 7
        local effect = 2
        local buff_time = 625
        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
