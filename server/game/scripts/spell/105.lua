-- spell: 삼성백열장
-- 삼성백열장: deal magic damage to the selected target.
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = 796
        local mp = 420
        local sound = 8
        local effect = 86
        spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
