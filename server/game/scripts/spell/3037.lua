-- spell: 진뢰격참주'첨
-- 진뢰격참주'첨 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local damage = 1930
        local mp = 400
        local sound = 58
        local effect = 30
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
