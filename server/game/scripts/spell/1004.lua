-- spell: 유인
-- 유인 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        if not you:is(OBJECT_TYPE.MOB) then
            return me:message('걸리지 않습니다.')
        end

        local mp = 50
        local sound = 2
        local effect = 88
        if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
            you:target(me)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
