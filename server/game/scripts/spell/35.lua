-- spell: 용천제육격
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = math.random(78000, 80000)
        local mp = 100
        local sound = 8
        local effect = 51
        spell.damage(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect })
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
