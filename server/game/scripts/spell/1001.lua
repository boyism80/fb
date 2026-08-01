-- spell: 신검합일
-- 신검합일 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp, message)
        local mp = 60
        local dam = 10
        spell.weapon_damage(me, mp, message, dam)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
