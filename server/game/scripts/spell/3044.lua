-- spell: 삼매진화
-- 삼매진화 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local damage = math.floor(me:mp() * 1.5)
        local mp = me:mp()
        local sound = 73
        local effect = 8
        local delay = 60
        if spell.damage_near_target(me, you, sp, { damage = damage, mp = mp, sound = sound, effect = effect }) and me:is(OBJECT_TYPE.CHARACTER) then
            me:spell(sp):delay(delay)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
