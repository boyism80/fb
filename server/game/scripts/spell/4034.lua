-- spell: 태양의기원
-- 태양의기원 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local hp = 1000
        local mp = 240
        local sound = 76
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
