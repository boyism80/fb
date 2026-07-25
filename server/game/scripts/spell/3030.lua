-- spell: 화열참주'첨
-- 화열참주'첨 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local damage = 720
        local mp = 330
        local sound = 42
        local effect = 47
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
