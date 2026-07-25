-- spell: 구름의기원
-- 구름의기원 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local hp = 500
        local mp = 120
        local sound = 66
        local effect = 66
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
