-- spell: 대지의힘
-- 대지의힘 캐스팅
local spell = require('lib.spell')

-- 대지의힘 버프 효과

-- 대지의힘 버프 해제 효과

return {
    ON_CAST = function(me, you, sp)
        if not you:is(OBJECT_TYPE.CHARACTER) then
            return me:message('걸리지 않습니다.')
        end

        local mp = 30
        local sound = 21
        local effect = 11
        local buff_time = 5
        if spell.buff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
            you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:buff_str(me:buff_str()+3)
    end,

    ON_UNBUFF = function(me, sp)
        me:buff_str(me:buff_str()-3)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
