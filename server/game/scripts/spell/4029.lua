-- spell: 하늘의기원
-- 하늘의기원 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local hp = 200
        local mp = 120
        local sound = 3
        local effect = 5
        spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
