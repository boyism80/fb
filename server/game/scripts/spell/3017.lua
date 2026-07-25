-- spell: 진뢰격참주
-- 진뢰격참주 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = 1950
        local mp = 330
        local sound = 58
        local effect = 30
        spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
