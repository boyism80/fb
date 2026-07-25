-- spell: 대지의기원
-- 대지의기원 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local hp = 50
        local mp = 50
        local sound = 3
        local effect = 5
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
