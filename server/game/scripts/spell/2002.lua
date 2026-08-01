-- spell: 금수
-- 금수 캐스팅
local spell = require('lib.spell')

-- 금수 버프 효과
-- 금수 버프 해제 효과

return {
    on_cast = function(me, sp, name)
        local mobs = {
            ['고양이'] = 183 + 0x7FFF,
            ['새끼돼지'] = name2mob('꼬마돼지'):look(),
            ['돼지'] = name2mob('돼지'):look(),
            ['강아지'] = name2mob('강아지'):look(),
            ['누렁이'] = name2mob('누렁이'):look()
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
