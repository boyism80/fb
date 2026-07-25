-- spell: 자무주'첨
-- 자무주'첨 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = 300
        local mp = 180
        local sound = 45
        local effect = 23
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
