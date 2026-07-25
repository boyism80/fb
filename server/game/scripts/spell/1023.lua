-- spell: 운공체식
-- 운공체식 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local hp = 10000
        local mp = 10000
        local sound = 61
        local effect = 166
        spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
