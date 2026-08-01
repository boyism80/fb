-- spell: 해독
-- 해독 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local mp = 30
        local sound = 63
        local effect = 22
        if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
            you:unbuff('중독')
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
