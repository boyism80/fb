-- spell: 시력회복
-- 시력회복 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local mp = 40
        local sound = 75
        local effect = 10
        if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
            you:unbuff('절망')
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
