-- spell: 야수
-- 야수 캐스팅
local spell = require('lib.spell')

-- 야수 버프 효과
-- 야수 버프 해제 효과

return {
    ON_CAST = function(me, sp, name)
        local mobs = {
            ['여우'] = name2mob('여우'):look(),
            ['늑대'] = name2mob('늑대'):look(),
        }

        local mp = 30
        local effect = 3
        local sound = 25
        local buff_time = 60
        spell.disguise(me, mobs, name, sp, {mp=mp, sound=sound, effect=effect, buff_time=buff_time})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    ON_UNBUFF = function(me, sp)
        me:mimic(nil)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
