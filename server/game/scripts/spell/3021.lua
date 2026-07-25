-- spell: 극진뢰격참주
-- 극진뢰격참주 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = 3580 -- + me:int()*5
        local mp = 360
        local sound = 59
        local effect = 30
        spell.damage(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
