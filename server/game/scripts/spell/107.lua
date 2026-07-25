-- spell: 오성백열장
-- 오성백열장: deal magic damage to the selected target.
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = 1380
        local mp = 700
        local sound = 8
        local effect = 86
        spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
