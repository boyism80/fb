-- spell: 흡성대법
-- 흡성대법 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = 3000
        local mp = 60
        local sound = 38
        local effect = 44

        if not you:is(OBJECT_TYPE.MOB) then
        	return me:message('걸리지 않습니다.')
        end

        if you:maxhp() > damage then
        	return me:message('걸리지 않습니다.')
        end

        me:heal(math.min(damage, you:hp()))
        spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
