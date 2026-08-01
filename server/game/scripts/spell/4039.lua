-- spell: 신령의기원'첨
-- 신령의기원'첨 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local mp = 1000
        local sound = 97
        local effect = 63
        local hp = 10000
        if not spell.heal_group(me, sp, {hp=hp, mp=mp, sound=sound, effect=effect}) then
            return
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
