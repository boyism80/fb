-- spell: 진뢰격참주
-- 진뢰격참주 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = 1950
        local mp = 330
        local sound = 58
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
