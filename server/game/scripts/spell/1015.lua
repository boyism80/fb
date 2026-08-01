-- spell: 동귀어진
-- 동귀어진 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local effect = 9
        local sound = 18
        local hp = me:hp() - 10
        local mp = 10
        local damage = me:hp()*2
        local message = '동귀어진'

        local front = me:front(OBJECT_TYPE.LIFE)
        if not spell.attack_cast(me, front, sp, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
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
