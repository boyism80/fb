-- spell: 검신검귀
-- 검신검귀 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp, message)
        local mp = 60
        local dam = 20
        spell.weapon_damage(me, mp, message, dam)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
