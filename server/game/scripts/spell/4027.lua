-- spell: 대지의기원
-- 대지의기원 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local hp = 50
        local mp = 50
        local sound = 3
        local effect = 5
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
