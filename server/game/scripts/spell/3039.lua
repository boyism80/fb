-- spell: 극진자천무주'첨
-- 극진자천무주'첨 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = 3560
        local mp = 470
        local sound = 49
        local effect = 26
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
