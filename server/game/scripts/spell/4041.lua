-- spell: 봉황의기원
-- 봉황의기원 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local hp = 30000
        local mp = 10000
        local sound = 505
        local effect = 165
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
