-- spell: 백열주'첨
-- 백열주'첨 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local damage = 300
        local mp = 180
        local sound = 50
        local effect = 40
        spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
