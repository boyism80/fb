-- spell: 구름의기원
-- 구름의기원 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local hp = 500
        local mp = 120
        local sound = 66
        local effect = 66
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
