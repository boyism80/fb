-- spell: 백호의희원'첨
-- 백호의희원'첨 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local sound = 95
        local effect = 64
        local ok, delay = spell.heal_group_baekho(me, sp, { sound = sound, effect = effect })
        if not ok then
            return
        end
        me:spell(sp):delay(delay)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
