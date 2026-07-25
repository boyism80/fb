-- spell: 지진
-- 지진 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = 1500
        local mp = 580
        local sound = 82
        local effect = 17
        spell.damage_near_target(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
