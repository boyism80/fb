-- spell: 경수
-- 경수 캐스팅
local spell = require('lib.spell')

-- 경수 버프 효과
-- 경수 버프 해제 효과

return {
    on_cast = function(me, sp, name)
        local mobs = {
            ['토끼'] = name2mob('토끼'):look(),
            ['다람쥐'] = name2mob('다람쥐'):look(),
        }

        local mp = 30
        local effect = 3
        local sound = 25
        local buff_time = 60
        spell.disguise(me, mobs, name, sp, {mp=mp, sound=sound, effect=effect, buff_time=buff_time})
    end,

    -- on_buff = function(me, sp)
    -- end,

    on_unbuff = function(me, sp)
        me:mimic(nil)
    end,

    -- on_concast = function(me, sp)
    -- end
}
