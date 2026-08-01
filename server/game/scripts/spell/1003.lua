-- spell: 대력검신
-- 대력검신 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp, message)
        local mp = 60
        local dam = 30
        spell.weapon_damage(me, mp, message, dam)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
