-- spell: 뢰격주'첨
-- 뢰격주'첨 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = 510
        local mp = 250
        local sound = 56
        local effect = 28
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
