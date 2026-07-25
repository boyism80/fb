-- spell: 망각
-- 망각 캐스팅
local spell = require('lib.spell')

-- 망각 버프 효과

-- 망각 버프 해제 효과

return {
    ON_CAST = function(me, you, sp)
        if not you:is(OBJECT_TYPE.MOB) then
            return me:message('걸리지 않습니다.')
        end

        local mp = 60
        local sound = 32
        local effect = 34
        local buff_time = 4
        if spell.buff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
            you:oblivion(me)
        	you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:target(nil)
    end,

    ON_UNBUFF = function(me, sp)
        me:oblivion(nil)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
