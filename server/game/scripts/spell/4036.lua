-- spell: 생명의기원
-- 생명의기원 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local hp = 5000
        local mp = 300
        local sound = 98
        local effect = 65
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
