-- spell: 화열참주'첨
-- 화열참주'첨 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = 720
        local mp = 330
        local sound = 42
        local effect = 47
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
