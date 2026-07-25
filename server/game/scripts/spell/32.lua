-- spell: 용천제삼격
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = math.random(18000, 20000)
        local mp = 100
        local sound = 8
        local effect = 51
        spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
