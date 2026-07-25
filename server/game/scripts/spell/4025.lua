-- spell: 누리의기원
-- 누리의기원 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local hp = 50
        local mp = 30
        local sound = 3
        local effect = 5
        spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
