-- spell: 검신검귀
-- 검신검귀 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp, message)
        local mp = 60
        local dam = 20
        spell.weapon_damage(me, mp, message, dam)
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
