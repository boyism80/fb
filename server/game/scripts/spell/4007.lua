-- spell: 파혼술
-- 파혼술 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local mp = 30
        local sound = 77
        local effect = 49
        if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
            you:unbuff('혼마술')
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
