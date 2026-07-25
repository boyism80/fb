-- spell: 파혼술
-- 파혼술 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local mp = 30
        local sound = 77
        local effect = 49
        if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
            you:unbuff('혼마술')
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
