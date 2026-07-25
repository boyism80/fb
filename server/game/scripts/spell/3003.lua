-- spell: 자무주
-- 자무주 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = 320
        local mp = 120
        local sound = 45
        local effect = 23
        spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
