-- spell: 자천무주'첨
-- 자천무주'첨 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = 720
        local mp = 330
        local sound = 47
        local effect = 25
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
