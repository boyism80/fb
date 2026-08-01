-- spell: 운공체식
-- 운공체식 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local hp = 10000
        local mp = 10000
        local sound = 61
        local effect = 166
        spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
