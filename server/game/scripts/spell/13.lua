-- spell: 자동투명
-- 자동투명 캐스팅
local spell = require('lib.spell')

-- 자동투명 버프 효과

-- 자동투명 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local buff_time = 180
        if me:isbuff(table.unpack(spell.relative_buff_name(sp:name()))) then
            return false
        end

        me:buff(sp, buff_time)
    end,

    ON_BUFF = function(me, sp)
        me:state(STATE.HALF_CLOACK)
    end,

    ON_UNBUFF = function(me, sp)
        me:state(STATE.NORMAL)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
