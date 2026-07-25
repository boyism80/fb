-- spell: 백열주
-- 백열주 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = 320
        local mp = 120
        local sound = 50
        local effect = 40
        spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
